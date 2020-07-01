//
// Created by Joshua Heinemann on 04.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx.h>
#include <sgx_defs.h>
#include <sgx/sgx_wamr_enclave_types.h>
#include <sgx_wamr_native_symbols_wrapper.h>
#include <sgx/faasm_sgx_error.h>
#include <wasm_export.h>
#include <string.h>
#include <sgx_thread.h>
#if(WASM_ENABLE_INTERP == 1)
#include <wasm_runtime.h>
#endif
#if(WASM_ENABLE_AOT == 1)
#include <aot_runtime.h>
#endif
_sgx_wamr_tcs_t* sgx_wamr_tcs = NULL;
sgx_thread_mutex_t mutex_sgx_wamr_tcs = SGX_THREAD_MUTEX_INITIALIZER;
extern "C"{
    typedef void(*os_print_function_t)(const char* msg);
    extern void os_set_print_function(os_print_function_t pf);
    extern int os_printf(const char* message, ...);
    extern uint32_t get_libc_builtin_export_apis(NativeSymbol** native_symbol_ptr);
    extern sgx_status_t SGX_CDECL ocall_printf(const char* msg);
    static unsigned int sgx_wamr_tcs_len;
    static char wamr_global_heap_buffer[FAASM_SGX_WAMR_HEAP_SIZE * 1024];
    static inline faasm_sgx_status_t _get_tcs_slot(unsigned int* thread_id){
        for(int i = 0; i < sgx_wamr_tcs_len; i++){
            if(sgx_wamr_tcs[i].module == NULL){
                *thread_id = i;
                return FAASM_SGX_SUCCESS;
            }
        }
        return FAASM_SGX_WAMR_UNABLE_TO_FIND_TCS_SLOT;
    }
    faasm_sgx_status_t sgx_wamr_enclave_call_function(const char* wasm_function_name, uint32_t wasm_function_argc, uint32_t* wasm_function_argv, unsigned int thread_id){
        wasm_function_inst_t wasm_function;
        if(!wasm_function_name)
            return FAASM_SGX_INVALID_PTR;
        if(thread_id >= sgx_wamr_tcs_len)
            return FAASM_SGX_INVALID_THREAD_ID;
        if(!(wasm_function = wasm_runtime_lookup_function(sgx_wamr_tcs[thread_id].module_inst,wasm_function_name,NULL)))
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        if(!(wasm_runtime_call_wasm(sgx_wamr_tcs[thread_id].exev_env,wasm_function,wasm_function_argc,wasm_function_argv))){
#if(WASM_ENABLE_INTERP == 1 && WASM_ENABLE_AOT == 0)
            ocall_printf(((WASMModuleInstance*) sgx_wamr_tcs[thread_id].module_inst)->cur_exception);
#elif(WASM_ENABLE_INTERP == 0 && WASM_ENABLE_AOT == 1)
            ocall_printf(((AOTModuleInstance*) sgx_wamr_tcs[thread_id].module_inst)->cur_exception);
#else
            ocall_printf(sgx_wamr_tcs[thread_id].module_inst->module_type == Wasm_Module_Bytecode? ((WASMModuleInstance*)sgx_wamr_tcs[thread_id].module_inst)->cur_exception : ((AOTModuleInstance*)sgx_wamr_tcs[thread_id].module_inst)->cur_exception);
#endif
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t sgx_wamr_enclave_load_module(const void* wasm_opcode_ptr, uint32_t wasm_opcode_size, unsigned int* thread_id){//TODO: CODE SIZE
        char module_error_buffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];
        faasm_sgx_status_t ret_val;
        memset(module_error_buffer, 0x0, sizeof(module_error_buffer));
        if(!wasm_opcode_ptr)
            return FAASM_SGX_INVALID_PTR;
        sgx_thread_mutex_lock(&mutex_sgx_wamr_tcs);
        if((ret_val = _get_tcs_slot(thread_id)) != FAASM_SGX_SUCCESS)
            return ret_val;//TODO: REALLOC
        sgx_wamr_tcs[*thread_id].module = (WASMModuleCommon*) 0x1;
        sgx_thread_mutex_unlock(&mutex_sgx_wamr_tcs);
        if(!(sgx_wamr_tcs[*thread_id].module = wasm_runtime_load((uint8_t*)wasm_opcode_ptr,wasm_opcode_size,module_error_buffer,sizeof(module_error_buffer)))){
            ocall_printf(module_error_buffer);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }
        if(!(sgx_wamr_tcs[*thread_id].module_inst = wasm_runtime_instantiate(sgx_wamr_tcs[*thread_id].module,(uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,(uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,module_error_buffer,sizeof(module_error_buffer)))){
            ocall_printf(module_error_buffer);
            return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
        }
        if(!(sgx_wamr_tcs[*thread_id].exev_env = wasm_runtime_create_exec_env(sgx_wamr_tcs[*thread_id].module_inst,FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE))){
            return FAASM_SGX_WAMR_EXECUTION_ENVIRONMENT_GENERATION_FAILED;
        }
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t sgx_wamr_enclave_init_wamr(const unsigned int thread_number) {
        os_set_print_function((os_print_function_t)ocall_printf);
        NativeSymbol* native_symbol_ptr = NULL;
        RuntimeInitArgs wamr_runtime_init_args;
        if((sgx_wamr_tcs = (_sgx_wamr_tcs_t*) calloc(thread_number, sizeof(_sgx_wamr_tcs_t))) == NULL){
            return FAASM_SGX_OUT_OF_MEMORY;
        }
        sgx_wamr_tcs_len = thread_number;
        memset(&wamr_runtime_init_args, 0x0, sizeof(wamr_runtime_init_args));
        wamr_runtime_init_args.mem_alloc_type = Alloc_With_Pool;
        wamr_runtime_init_args.mem_alloc_option.pool.heap_buf = wamr_global_heap_buffer;
        wamr_runtime_init_args.mem_alloc_option.pool.heap_size = sizeof(wamr_global_heap_buffer);
        wamr_runtime_init_args.native_module_name = "env";
        wamr_runtime_init_args.n_native_symbols = get_libc_builtin_export_apis(&native_symbol_ptr);
        wamr_runtime_init_args.native_symbols = native_symbol_ptr;
        if(!wasm_runtime_full_init(&wamr_runtime_init_args))
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        return FAASM_SGX_SUCCESS;
    }
};