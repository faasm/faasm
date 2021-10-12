#include <sgx/attestation.h>
#include <sgx/enclave_config.h>
#include <sgx/enclave_types.h>
#include <sgx/error.h>
#include <sgx/native.h>
#include <sgx/native_symbols_wrapper.h>
#include <sgx/rw_lock.h>

#include <iwasm/include/wasm_export.h>
#include <libcxx/cstdlib>
#include <sgx.h>
#include <sgx_defs.h>
#include <sgx_thread.h>
#include <tlibc/mbusafecrt.h>

#include <iwasm/aot/aot_runtime.h>

#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define WASM_ENTRY_FUNC "_start"

// Implementation of the exposed Ecalls API. The code in this file is meant to
// run _inside_ the enclave, thus is the only bit of code that actually
// interacts with the WAMR runtime.
extern "C"
{

#ifdef FAASM_SGX_DEBUG
    // Print functions
    typedef void (*os_print_function_t)(const char* msg);
    extern void os_set_print_function(os_print_function_t pf);
    extern sgx_status_t SGX_CDECL ocall_printf(const char* msg);
    extern int os_printf(const char* message, ...);
#endif

    // Print function wrapper
    void SGX_DEBUG_LOG(const char* message)
    {
#ifdef FAASM_SGX_DEBUG
        ocall_printf(message);
#else
        ;
#endif
    }

    static rwlock_t _rwlock_faasm_sgx_tcs_realloc = { 0 };

    _faasm_sgx_tcs_t** faasm_sgx_tcs = NULL;
    static uint32_t _faasm_sgx_tcs_len;
    static sgx_thread_mutex_t _mutex_faasm_sgx_tcs =
      SGX_THREAD_MUTEX_INITIALIZER;

    static uint8_t _wamr_global_heap_buffer[FAASM_SGX_WAMR_HEAP_SIZE];

    static __attribute__((always_inline)) faasm_sgx_status_t __get_tcs_slot(
      uint32_t* thread_id)
    {
        _faasm_sgx_tcs_t** temp_ptr;
        uint32_t temp_len, i = 0;
        sgx_thread_mutex_lock(&_mutex_faasm_sgx_tcs);

        // Search for an unused TCS slot
        for (; i < _faasm_sgx_tcs_len; i++) {
            if (!faasm_sgx_tcs[i]) {
                goto _FAASM_SGX_TCS_BLOCK_SLOT_AND_ALLOCATE_MEMORY;
            }
        }

        // In case that all TCS slots are in use, the existing TCS structure
        // will be dynamically extended
        temp_len = (_faasm_sgx_tcs_len << 1);
        write_lock(&_rwlock_faasm_sgx_tcs_realloc);

        if (!(temp_ptr = (_faasm_sgx_tcs_t**)realloc(
                faasm_sgx_tcs, (temp_len * sizeof(_faasm_sgx_tcs_t*))))) {
            // Revert all changes due to an memory-error
            write_unlock(&_rwlock_faasm_sgx_tcs_realloc);
            sgx_thread_mutex_unlock(&_mutex_faasm_sgx_tcs);
            SGX_DEBUG_LOG("OOM on get TCS slot\n");
            return FAASM_SGX_OUT_OF_MEMORY;
        }

        // Have to zero out new memory because realloc can refer to already used
        // memory, hence faasm_sgx_tcs[X].module might not be zero
        memset((void*)(temp_ptr + _faasm_sgx_tcs_len),
               0x0,
               (temp_len - _faasm_sgx_tcs_len) * sizeof(_faasm_sgx_tcs_t*));

        // Update faasm_sgx_tcs ptr & len
        faasm_sgx_tcs = temp_ptr;
        write_unlock(&_rwlock_faasm_sgx_tcs_realloc);
        _faasm_sgx_tcs_len = temp_len;

    // Block Faasm-SGX TCS slot and reserve memory for Faasm-SGX TCS structure
    _FAASM_SGX_TCS_BLOCK_SLOT_AND_ALLOCATE_MEMORY:
        faasm_sgx_tcs[i] = _FAASM_SGX_TCS_SLOT_IN_USE;
        sgx_thread_mutex_unlock(&_mutex_faasm_sgx_tcs);
        if (!(faasm_sgx_tcs[i] = (_faasm_sgx_tcs_t*)calloc(
                sizeof(_faasm_sgx_tcs_t), sizeof(uint8_t)))) {
            SGX_DEBUG_LOG("OOM on allocate TCS structure\n");
            return FAASM_SGX_OUT_OF_MEMORY;
        }

        *thread_id = i;
        return FAASM_SGX_SUCCESS;
    }

    // Execute the main function
    faasm_sgx_status_t faasm_sgx_enclave_call_function(const uint32_t thread_id)
    {
        // Check if thread_id is in range
        if (thread_id >= _faasm_sgx_tcs_len) {
            return FAASM_SGX_INVALID_THREAD_ID;
        }

        // Get Faasm-SGX TCS slot using thread_id
        _FAASM_SGX_TCS_LOAD_SLOT(tcs_ptr, thread_id);

        // Get function to execute
        WASMFunctionInstanceCommon* wasm_func;
        if (!(wasm_func = wasm_runtime_lookup_function(
                tcs_ptr->module_inst, WASM_ENTRY_FUNC, NULL))) {
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        }

        // Create an execution environment and call the wasm function
        if (!(aot_create_exec_env_and_call_function(
              (AOTModuleInstance*)tcs_ptr->module_inst,
              (AOTFunctionInstance*)wasm_func,
              0x0,
              0x0))) {

            // First, check if the _FAASM_SGX_ERROR_PREFIX is set
            // If so, then obtain and return the faasm_sgx_status_t error code
            // If not, just print the exception and return the matching
            // Faasm-SGX error code
            if (!memcmp(
                  ((AOTModuleInstance*)tcs_ptr->module_inst)->cur_exception,
                  _FAASM_SGX_ERROR_PREFIX,
                  sizeof(_FAASM_SGX_ERROR_PREFIX))) {

                return *((faasm_sgx_status_t*)&(
                  ((AOTModuleInstance*)tcs_ptr->module_inst)
                    ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]));
            }

            SGX_DEBUG_LOG(
              ((AOTModuleInstance*)tcs_ptr->module_inst)->cur_exception);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }
        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t faasm_sgx_enclave_unload_module(const uint32_t thread_id)
    {
        // Important possibility check before unloading a module
        if (thread_id >= _faasm_sgx_tcs_len) {
            return FAASM_SGX_INVALID_THREAD_ID;
        }

        // Check if the linked module is already unloaded
        read_lock(&_rwlock_faasm_sgx_tcs_realloc);
        if (faasm_sgx_tcs[thread_id] == _FAASM_SGX_TCS_SLOT_FREE) {
            read_unlock(&_rwlock_faasm_sgx_tcs_realloc);
            return FAASM_SGX_MODULE_NOT_LOADED;
        }

        _faasm_sgx_tcs_t* tcs_ptr = faasm_sgx_tcs[thread_id];
        read_unlock(&_rwlock_faasm_sgx_tcs_realloc);

        // Unload the module and release the TCS slot
        wasm_runtime_unload(tcs_ptr->module);
        wasm_runtime_deinstantiate(tcs_ptr->module_inst);
        free(tcs_ptr->wasm_opcode);
        _FAASM_SGX_TCS_FREE_SLOT(thread_id);
        free(tcs_ptr);

        return FAASM_SGX_SUCCESS;
    }

    // Load the provided web assembly module to the enclave's runtime
    faasm_sgx_status_t faasm_sgx_enclave_load_module(
      const void* wasm_opcode_ptr,
      const uint32_t wasm_opcode_size,
      uint32_t* thread_id)
    {
        char wamr_error_buffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];
        faasm_sgx_status_t return_value;

        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasm_opcode_size) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (!wasm_opcode_ptr) {
            return FAASM_SGX_INVALID_PTR;
        }

        // Get Faasm-SGX TCS slot for further use and set thread-specific
        // thread_id
        if ((return_value = __get_tcs_slot(thread_id)) != FAASM_SGX_SUCCESS) {
            return return_value;
        }

        // Initialize Faasm-SGX TCS slot and copy wasm code
        _FAASM_SGX_TCS_LOAD_SLOT(tcs_ptr, *thread_id);
        uint8_t* wasm_buffer_ptr =
          (uint8_t*)calloc(wasm_opcode_size, sizeof(uint8_t));
        if (!wasm_buffer_ptr) {
            // Revert all changes due to the out of memory error
            _FAASM_SGX_TCS_FREE_SLOT(*thread_id);
            free(tcs_ptr);
            SGX_DEBUG_LOG("Unable to allocate memory for wasmBytes\n");
            return FAASM_SGX_OUT_OF_MEMORY;
        }
        tcs_ptr->wasm_opcode = wasm_buffer_ptr;
        memcpy(tcs_ptr->wasm_opcode, wasm_opcode_ptr, wasm_opcode_size);

        // Load the WASM module
        if (!(tcs_ptr->module = wasm_runtime_load(tcs_ptr->wasm_opcode,
                                                  wasm_opcode_size,
                                                  wamr_error_buffer,
                                                  sizeof(wamr_error_buffer)))) {
            // Revert all changes due to a module load error
            free(tcs_ptr->wasm_opcode);
            _FAASM_SGX_TCS_FREE_SLOT(*thread_id);
            free(tcs_ptr);
            SGX_DEBUG_LOG(wamr_error_buffer);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }

        // Instantiate the WASM module
        if (!(tcs_ptr->module_inst = wasm_runtime_instantiate(
                tcs_ptr->module,
                (uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,
                (uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,
                wamr_error_buffer,
                sizeof(wamr_error_buffer)))) {
            // Revert all changes due to a module instantiate error
            free(tcs_ptr->wasm_opcode);
            wasm_runtime_unload(tcs_ptr->module);
            _FAASM_SGX_TCS_FREE_SLOT(*thread_id);
            free(tcs_ptr);
            SGX_DEBUG_LOG(wamr_error_buffer);
            return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }

    // Set up the WAMR runtime, and initialise all enclave-related
    // variables. Currently, this happens _once_ per Faasm instance. This is,
    // we only run one enclave per Faasm instance.
    faasm_sgx_status_t faasm_sgx_enclave_init_wamr(void)
    {
        os_set_print_function((os_print_function_t)SGX_DEBUG_LOG);

        // Initialize FAASM-SGX TCS
        _faasm_sgx_tcs_len = FAASM_SGX_INIT_TCS_SLOTS;
        if (!(faasm_sgx_tcs = (_faasm_sgx_tcs_t**)calloc(
                FAASM_SGX_INIT_TCS_SLOTS, sizeof(_faasm_sgx_tcs_t*)))) {
            SGX_DEBUG_LOG("OOM error initialising TCS\n");
            return FAASM_SGX_OUT_OF_MEMORY;
        }

        // Initialise the WAMR runtime
        RuntimeInitArgs wamr_rte_args;
        memset(&wamr_rte_args, 0x0, sizeof(wamr_rte_args));
        wamr_rte_args.mem_alloc_type = Alloc_With_Pool;
        wamr_rte_args.mem_alloc_option.pool.heap_buf =
          (void*)_wamr_global_heap_buffer;
        wamr_rte_args.mem_alloc_option.pool.heap_size =
          sizeof(_wamr_global_heap_buffer);

        if (!wasm_runtime_full_init(&wamr_rte_args)) {
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Set up native symbols
        sgx::initialiseSGXWAMRNatives();

        return FAASM_SGX_SUCCESS;
    }
}
