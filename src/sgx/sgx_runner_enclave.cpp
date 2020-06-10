//
// Created by Joshua Heinemann on 04.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx.h>
#include <sgx_defs.h>
#include <sgx/faasm_sgx_error.h>
#include <string.h>
#include <wasm_export.h>

#ifndef FAASM_SGX_WAMR_HEAP_SIZE
#define FAASM_SGX_WAMR_HEAP_SIZE 256 //Default: 256KB
#endif

#ifndef FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE
#define FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128
#endif

#ifndef FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE 8196
#endif

#ifndef FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE
#define FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE 8196
#endif

extern "C"{
    extern sgx_status_t SGX_CDECL ocall_printf(const char* message);
    static char wamr_global_heap_buffer[FAASM_SGX_WAMR_HEAP_SIZE * 1024];
    static wasm_exec_env_t wamr_global_exec_env;
    static wasm_module_inst_t wamr_global_module_instance;
    void ocall_printf_wrapper(wasm_exec_env_t exec_env, const char* msg){
        ocall_printf(msg); //TODO: IMPROVE PRINTF
    }
    faasm_sgx_status_t enclave_call_function(const char* wasm_function_name, uint32_t wasm_function_argc, uint32_t* wasm_function_argv){
        wasm_function_inst_t wasm_function;
        if(!wasm_function_name)
            return FAASM_SGX_INVALID_PTR;
        if(!(wasm_function = wasm_runtime_lookup_function(wamr_global_module_instance,wasm_function_name,NULL)))
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        if(!(wasm_runtime_call_wasm(wamr_global_exec_env,wasm_function,wasm_function_argc,wasm_function_argv))){
            const char* error = wasm_runtime_get_exception(wamr_global_module_instance); //TODO: MAKE THIS MORE PRETTY
            ocall_printf(error);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t enclave_load_module(void* wasm_opcode_ptr, uint32_t wasm_opcode_size){
        wasm_module_t wasm_module;
        char module_error_buffer[FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE];
        memset(module_error_buffer, 0x0, sizeof(module_error_buffer));
        if(!wasm_opcode_ptr)
            return FAASM_SGX_INVALID_PTR;
        if(!(wasm_module = wasm_runtime_load((uint8_t*)wasm_opcode_ptr,wasm_opcode_size,module_error_buffer,sizeof(module_error_buffer)))){
            ocall_printf(module_error_buffer);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }
        if(!(wamr_global_module_instance = wasm_runtime_instantiate(wasm_module,(uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,(uint32_t)FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,module_error_buffer,sizeof(module_error_buffer)))){
            ocall_printf(module_error_buffer);
            return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
        }
        if(!(wamr_global_exec_env = wasm_runtime_create_exec_env(wamr_global_module_instance,FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE))){
            return FAASM_SGX_WAMR_EXECUTION_ENVIRONMENT_GENERATION_FAILED;
        }
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t enclave_init_wamr(void) {
        static NativeSymbol wamr_runtime_symbols[] = {
                {"printf",(void*)ocall_printf_wrapper,"($)"}
        };
        RuntimeInitArgs wamr_runtime_init_args;
        memset(&wamr_runtime_init_args, 0x0, sizeof(wamr_runtime_init_args));
        wamr_runtime_init_args.mem_alloc_type = Alloc_With_Pool;
        wamr_runtime_init_args.mem_alloc_option.pool.heap_buf = wamr_global_heap_buffer;
        wamr_runtime_init_args.mem_alloc_option.pool.heap_size = sizeof(wamr_global_heap_buffer);
        wamr_runtime_init_args.native_module_name = "env";
        wamr_runtime_init_args.native_symbols = wamr_runtime_symbols;
        wamr_runtime_init_args.n_native_symbols = sizeof(wamr_runtime_symbols) / sizeof(NativeSymbol);
        if(!wasm_runtime_full_init(&wamr_runtime_init_args))
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        return FAASM_SGX_SUCCESS;
    }
};