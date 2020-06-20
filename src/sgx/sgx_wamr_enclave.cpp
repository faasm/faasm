//
// Created by Joshua Heinemann on 04.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx.h>
#include <sgx_defs.h>
#include <sgx_wamr_native_symbols_wrapper.h>
#include <sgx/faasm_sgx_error.h>
#include <wasm_export.h>
#include <string.h>
#if(WASM_ENABLE_INTERP != 0)
#include <wasm_runtime.h>
#endif
#if(WASM_ENABLE_AOT != 1)
#include <aot_runtime.h>
#endif

extern "C"{
    typedef void(*os_print_function_t)(const char* msg);
    extern void os_set_print_function(os_print_function_t pf);
    extern int os_printf(const char* message, ...);
    extern uint32_t get_libc_builtin_export_apis(NativeSymbol** native_symbol_ptr);
    extern sgx_status_t SGX_CDECL ocall_printf(const char* msg);
    static char wamr_global_heap_buffer[FAASM_SGX_WAMR_HEAP_SIZE * 1024];
    static wasm_exec_env_t wamr_global_exec_env;
    static wasm_module_inst_t wamr_global_module_instance;
    faasm_sgx_status_t enclave_call_function(const char* wasm_function_name, uint32_t wasm_function_argc, uint32_t* wasm_function_argv){
        wasm_function_inst_t wasm_function;
        if(!wasm_function_name)
            return FAASM_SGX_INVALID_PTR;
        if(!(wasm_function = wasm_runtime_lookup_function(wamr_global_module_instance,wasm_function_name,NULL)))
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        if(!(wasm_runtime_call_wasm(wamr_global_exec_env,wasm_function,wasm_function_argc,wasm_function_argv))){
#if(WASM_ENABLE_INTERP == 1 && WASM_ENABLE_AOT == 0)
            ocall_printf(((WASMModuleInstance*) wamr_global_module_instance)->cur_exception);
#elif(WASM_ENABLE_INTERP == 0 && WASM_ENABLE_AOT == 1)
            ocall_printf(((AOTModuleInstance*) wamr_global_module_instance)->cur_exception);
#else
            ocall_printf(wamr_global_module_instance->module_type == Wasm_Module_Bytecode? ((WASMModuleInstance*)wamr_global_module_instance)->cur_exception : ((AOTModuleInstance*)wamr_global_module_instance)->cur_exception);
#endif
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }
        return FAASM_SGX_SUCCESS;
    }
    faasm_sgx_status_t enclave_load_module(const void* wasm_opcode_ptr, uint32_t wasm_opcode_size){
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
        os_set_print_function((os_print_function_t)ocall_printf);
        NativeSymbol* native_symbol_ptr = NULL;
        RuntimeInitArgs wamr_runtime_init_args;
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