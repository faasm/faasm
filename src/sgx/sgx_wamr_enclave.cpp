#include <sgx.h>
#include <sgx_defs.h>
#include <sgx_thread.h>
#include <sgx/faasm_sgx_error.h>
#include <sgx/sgx_wamr_enclave_types.h>
#include <sgx/sgx_wamr_native_symbols_wrapper.h>
#include <sgx/rw_lock.h>
#include <tlibc/mbusafecrt.h>
#include <libcxx/cstdlib>
#include <iwasm/include/wasm_export.h>
#include <iwasm/aot/aot_runtime.h>
#include <iwasm/interpreter/wasm_runtime.h>


#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_attestation.h>
#endif

#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

extern "C" {

#if(FAASM_SGX_DEBUG)
// Print functions
typedef void(*os_print_function_t)(const char *msg);
extern void os_set_print_function(os_print_function_t pf);
extern sgx_status_t SGX_CDECL ocall_printf(const char *msg);
extern int os_printf(const char *message, ...);
#endif

// WAMR native symbols
extern NativeSymbol faasm_sgx_native_symbols[FAASM_SGX_NATIVE_SYMBOLS_LEN];
extern NativeSymbol faasm_sgx_wasi_symbols[FAASM_SGX_WASI_SYMBOLS_LEN];

#if(FAASM_SGX_ATTESTATION)
#define INCREMENT_MSG_ID() \
__sync_fetch_and_add(&_sgx_wamr_msg_id, 1)
extern sgx_status_t SGX_CDECL ocall_init_crt(faasm_sgx_status_t *returnValue);
extern sgx_status_t SGX_CDECL ocall_send_msg(faasm_sgx_status_t *returnValue, sgx_wamr_msg_t *msg, uint32_t msg_len);
static uint8_t _sgx_wamr_msg_id = 0;
#endif

#if(FAASM_SGX_ATTESTATION || FAASM_SGX_WHITELISTING)
__thread uint32_t tls_thread_id;
#endif

_faasm_sgx_tcs_t *faasm_sgx_tcs = NULL;
static uint32_t _faasm_sgx_tcs_len;
static sgx_thread_mutex_t _mutex_faasm_sgx_tcs = SGX_THREAD_MUTEX_INITIALIZER;
static rwlock_t _rwlock_faasm_sgx_tcs_realloc = {0};
static uint8_t _wamr_global_heap_buffer[FAASM_SGX_WAMR_HEAP_SIZE * 1024];

#if(FAASM_SGX_ATTESTATION)
static inline faasm_sgx_status_t __get_response_msg(const uint32_t thread_id, sgx_wamr_msg_t **response_ptr) {
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);
    *response_ptr = *faasm_sgx_tcs[thread_id].response_ptr;
    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
    return FAASM_SGX_SUCCESS;
}

static faasm_sgx_status_t
recv_msg(uint32_t thread_id, void **payload_ptr, uint32_t *payload_len) {//TODO: Maybe replace thread_id with __thread
    if (thread_id >= _faasm_sgx_tcs_len)
        return FAASM_SGX_INVALID_THREAD_ID;
    if (!payload_ptr || !payload_len)
        return FAASM_SGX_INVALID_PTR;
    faasm_sgx_status_t returnValue;
    sgx_wamr_msg_t *response_ptr;
    if ((returnValue = __get_response_msg(thread_id, &response_ptr)) != FAASM_SGX_SUCCESS)
        return returnValue;
    ///////////ENCRYPTION///////////
    //Implement me :P
    ///////////ENCRYPTION///////////
    *payload_ptr = (void *) response_ptr->payload;
    *payload_len = response_ptr->payload_len;
    return FAASM_SGX_SUCCESS;
}

static faasm_sgx_status_t send_msg(const void *payload_ptr, const uint32_t payload_len) {
    sgx_wamr_msg_t *msg_ptr;
    sgx_status_t sgxReturnValue;
    faasm_sgx_status_t returnValue;

    if (!payload_ptr) {
        return FAASM_SGX_INVALID_PTR;
    }

    if (!payload_len) {
        return FAASM_SGX_INVALID_PAYLOAD_LEN;
    }

    if (!(msg_ptr = (sgx_wamr_msg_t *) calloc((sizeof(sgx_wamr_msg_t) + payload_len), sizeof(uint8_t)))) {
#if(FAASM_SGX_DEBUG)
        ocall_printf("Enclave send_msg failed, OOM\n");
#endif
        return FAASM_SGX_OUT_OF_MEMORY;
    }

    msg_ptr->msg_id = INCREMENT_MSG_ID();
    ///////////ENCRYPTION///////////
    //Implement me :P
    ///////////ENCRYPTION///////////
    ///////////REMOVE IF ENCRYPTION WORKS///////////
    msg_ptr->payload_len = payload_len;
    memcpy(((uint8_t *) msg_ptr->payload), payload_ptr, payload_len);
    ///////////REMOVE IF ENCRYPTION WORKS///////////
    if ((sgxReturnValue = ocall_send_msg(&returnValue, msg_ptr, sizeof(sgx_wamr_msg_t) + msg_ptr->payload_len)) !=
        SGX_SUCCESS) {
        free(msg_ptr);
        return FAASM_SGX_OCALL_ERROR(sgxReturnValue);
    }
    free(msg_ptr);
    return returnValue;
}
#endif

static inline faasm_sgx_status_t __get_tcs_slot(uint32_t *thread_id) {
    _faasm_sgx_tcs_t *temp_ptr;
    uint32_t temp_len, i = 0;
    sgx_thread_mutex_lock(&_mutex_faasm_sgx_tcs);
    read_lock(&_rwlock_faasm_sgx_tcs_realloc); //TODO: Check threading
    for (; i < _faasm_sgx_tcs_len; i++) {
        if (faasm_sgx_tcs[i].module == NULL) {
            faasm_sgx_tcs[i].module = (WASMModuleCommon *) 0x1;
            sgx_thread_mutex_unlock(&_mutex_faasm_sgx_tcs);
            read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
            *thread_id = i;
            return FAASM_SGX_SUCCESS;
        }
    }

    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
    temp_len = (_faasm_sgx_tcs_len << 1);
    write_lock(&_rwlock_faasm_sgx_tcs_realloc);

    if ((temp_ptr = (_faasm_sgx_tcs_t *) realloc(faasm_sgx_tcs, (temp_len * sizeof(_faasm_sgx_tcs_t)))) != NULL) {
        //Have to zero out new memory because realloc can refer to already used memory
        memset(
                (void *) (temp_ptr + _faasm_sgx_tcs_len),
                0x0,
                (temp_len - _faasm_sgx_tcs_len) * sizeof(_faasm_sgx_tcs_t)
        );

        faasm_sgx_tcs = temp_ptr;
        _faasm_sgx_tcs_len = temp_len;
        faasm_sgx_tcs[i].module = (WASMModuleCommon *) 0x1;
        write_unlock(&_rwlock_faasm_sgx_tcs_realloc);
        sgx_thread_mutex_unlock(&_mutex_faasm_sgx_tcs);
        *thread_id = i;
        return FAASM_SGX_SUCCESS;
    }

    write_unlock(&_rwlock_faasm_sgx_tcs_realloc);
    sgx_thread_mutex_unlock(&_mutex_faasm_sgx_tcs);
#if(FAASM_SGX_DEBUG)
    ocall_printf("OOM on get TCS slot\n");
#endif
    return FAASM_SGX_OUT_OF_MEMORY;
}

faasm_sgx_status_t sgx_wamr_enclave_call_function(const uint32_t thread_id, const uint32_t func_id) {
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);
    auto wasm_module_inst_ptr = (WASMModuleInstance *) faasm_sgx_tcs[thread_id].module_inst;
    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);

    char func_id_str[33];
    if (func_id == 0) {
        sprintf_s(func_id_str, 33, "%s", ENTRY_FUNC_NAME);
    } else {
        sprintf_s(func_id_str, 33, "_faasm_func_%i", func_id);
    }

    if (thread_id >= _faasm_sgx_tcs_len) {
        return FAASM_SGX_INVALID_THREAD_ID;
    }

    wasm_function_inst_t wasm_function = wasm_runtime_lookup_function(
            (WASMModuleInstanceCommon *) wasm_module_inst_ptr,
            func_id_str,
            NULL
    );

    if (!wasm_function) {
#if(FAASM_SGX_DEBUG)
        os_printf("Could not find function: %s\n",func_id_str);
#endif

        return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
    }

#if(FAASM_SGX_ATTESTATION)
    tls_thread_id = thread_id;
#endif
    if (!(wasm_create_exec_env_and_call_function(wasm_module_inst_ptr, (WASMFunctionInstance *) wasm_function, 0,
                                                 0x0))) {
#if(WASM_ENABLE_INTERP == 1 && WASM_ENABLE_AOT == 0)
        if(!memcmp(wasm_module_inst_ptr->cur_exception,_WRAPPER_ERROR_PREFIX,sizeof(_WRAPPER_ERROR_PREFIX))){
                faasm_sgx_status_t returnValue = *((faasm_sgx_status_t*)&(wasm_module_inst_ptr->cur_exception[sizeof(_WRAPPER_ERROR_PREFIX)]));
                return returnValue;
        }
#if(FAASM_SGX_DEBUG)
        ocall_printf(wasm_module_inst_ptr->cur_exception);
#endif
#elif(WASM_ENABLE_INTERP == 0 && WASM_ENABLE_AOT == 1)
#if(FAASM_SGX_DEBUG)
        ocall_printf(((AOTModuleInstance*) wasm_module_inst_ptr)->cur_exception);
#endif
#else
#if(FAASM_SGX_DEBUG)
        ocall_printf(((WASMModuleInstanceCommon *) wasm_module_inst_ptr)->module_type == Wasm_Module_Bytecode
                     ? ((WASMModuleInstance *) wasm_module_inst_ptr)->cur_exception
                     : ((AOTModuleInstance *) wasm_module_inst_ptr)->cur_exception);
#endif
#endif
        return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
    }

    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t faasm_sgx_enclave_unload_module(const uint32_t thread_id) {
    // Important possibility check before unloading a module
    if (thread_id >= _faasm_sgx_tcs_len)
        return FAASM_SGX_INVALID_THREAD_ID;

    // Check if the linked module is already unloaded
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);
    if (faasm_sgx_tcs[thread_id].module == 0x0) {
        read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
        return FAASM_SGX_MODULE_NOT_LOADED;
    }

    // Unload the module and release the TCS slot
    wasm_runtime_unload(faasm_sgx_tcs[thread_id].module);
    wasm_runtime_deinstantiate(faasm_sgx_tcs[thread_id].module_inst);
    free(faasm_sgx_tcs[thread_id].wasm_opcode);
    faasm_sgx_tcs[thread_id].module = 0x0;
    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t faasm_sgx_enclave_load_module(const void *wasm_opcode_ptr, const uint32_t wasm_opcode_size,
        uint32_t *thread_id
#if(FAASM_SGX_ATTESTATION)
        , sgx_wamr_msg_t **response_ptr
#endif
) {
    char wamr_error_buffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];
    faasm_sgx_status_t return_value;

    // Check if passed wasm opcode size or wasm opcode ptr is zero
    if (!wasm_opcode_size) {
        return FAASM_SGX_INVALID_OPCODE_SIZE;
    }
#if(FAASM_SGX_ATTESTATION)
    if (!wasm_opcode_ptr || !response_ptr) {
#else
    if (!wasm_opcode_ptr) {
#endif
        return FAASM_SGX_INVALID_PTR;
    }

    // Get Faasm-SGX TCS slot for further use and set thread-specific thread_id
    if ((return_value = __get_tcs_slot(thread_id)) != FAASM_SGX_SUCCESS) {
        return return_value;
    }

#if(FAASM_SGX_WHITELISTING)
    tls_thread_id = *thread_id;
#endif

    // Initialize Faasm-SGX TCS slot and copy wasm code
    read_lock(&_rwlock_faasm_sgx_tcs_realloc);
#if(FAASM_SGX_ATTESTATION)
    faasm_sgx_tcs[*thread_id].response_ptr = response_ptr;
#endif
    uint8_t *wasm_buffer_ptr = (uint8_t *) calloc(wasm_opcode_size, sizeof(uint8_t));
    if (!wasm_buffer_ptr) {
        // Revert all changes due to the out of memory error
        faasm_sgx_tcs[*thread_id].module = 0x0;
        read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
#if(FAASM_SGX_DEBUG)
        ocall_printf("Unable to allocate memory for wasmBytes\n");
#endif
        return FAASM_SGX_OUT_OF_MEMORY;
    }
    faasm_sgx_tcs[*thread_id].wasm_opcode = wasm_buffer_ptr;
    memcpy(faasm_sgx_tcs[*thread_id].wasm_opcode, wasm_opcode_ptr, wasm_opcode_size);

    // Load the WASM module
    if (!(faasm_sgx_tcs[*thread_id].module = wasm_runtime_load(faasm_sgx_tcs[*thread_id].wasm_opcode, wasm_opcode_size,
            wamr_error_buffer,
            sizeof(wamr_error_buffer)))) {
        // Revert all changes due to a module load error
        free(faasm_sgx_tcs[*thread_id].wasm_opcode);
        faasm_sgx_tcs[*thread_id].module = 0x0;
        read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
#if(FAASM_SGX_DEBUG)
        ocall_printf(wamr_error_buffer);
#endif
        return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
    }

    // Instantiate the WASM module
    if (!(faasm_sgx_tcs[*thread_id].module_inst = wasm_runtime_instantiate(faasm_sgx_tcs[*thread_id].module,
            (uint32_t) FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,
            (uint32_t) FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,
            wamr_error_buffer,
            sizeof(wamr_error_buffer)))) {
        // Revert all changes due to a module instantiate error
        free(faasm_sgx_tcs[*thread_id].wasm_opcode);
        wasm_runtime_unload(faasm_sgx_tcs[*thread_id].module);
        faasm_sgx_tcs[*thread_id].module = 0x0;
        read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
#if(FAASM_SGX_DEBUG)
        ocall_printf(wamr_error_buffer);
#endif
        return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
    }

    read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t faasm_sgx_enclave_init_wamr(void) {
#if(FAASM_SGX_DEBUG)
    // Set our ocall_printf as default printf function if FAASM_SGX_DEBUG == 1
    os_set_print_function((os_print_function_t) ocall_printf);
#endif

    // Initialize FAASM-SGX TCS
    _faasm_sgx_tcs_len = FAASM_SGX_INIT_TCS_SLOTS;
    if (!(faasm_sgx_tcs = (_faasm_sgx_tcs_t *) calloc(FAASM_SGX_INIT_TCS_SLOTS, sizeof(_faasm_sgx_tcs_t)))) {
#if(FAASM_SGX_DEBUG)
        ocall_printf("OOM initialising WAMR\n");
#endif
        return FAASM_SGX_OUT_OF_MEMORY;
    }

    // Initialise the WAMR runtime
    RuntimeInitArgs wamr_rte_args;
    memset(&wamr_rte_args, 0x0, sizeof(wamr_rte_args));
    wamr_rte_args.mem_alloc_type = Alloc_With_Pool;
    wamr_rte_args.mem_alloc_option.pool.heap_buf = (void*) _wamr_global_heap_buffer;
    wamr_rte_args.mem_alloc_option.pool.heap_size = sizeof(_wamr_global_heap_buffer);
    if (!wasm_runtime_full_init(&wamr_rte_args)) {
        return FAASM_SGX_WAMR_RTE_INIT_FAILED;
    }

    // Set up native symbols
    wasm_native_register_natives("env", faasm_sgx_native_symbols,
            FAASM_SGX_NATIVE_SYMBOLS_LEN);
    wasm_native_register_natives(
            "wasi_snapshot_preview1", faasm_sgx_wasi_symbols,
            FAASM_SGX_WASI_SYMBOLS_LEN);


#if(FAASM_SGX_ATTESTATION)
    //Initialize necessary attestation stuff if FAASM-SGX Attestation extention is enabled
    sgx_status_t sgx_return_value;
    faasm_sgx_status_t return_value;

    if ((sgx_return_value = ocall_init_crt(&return_value)) != SGX_SUCCESS) {
        return FAASM_SGX_OCALL_ERROR(sgx_return_value);
    }
    
    if (return_value != FAASM_SGX_SUCCESS) {
        return return_value;
    }
#endif

    return FAASM_SGX_SUCCESS;
}
}