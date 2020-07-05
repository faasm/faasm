//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx_wamr_native_symbols_wrapper.h>
#include <iwasm/common/wasm_exec_env.h>
#include <iwasm/common/wasm_runtime_common.h>
#include <iwasm/interpreter/wasm_runtime.h>

extern "C"{
    extern int os_printf(const char* message, ...);
    static void sgx_wamr_example_native_symbol_wrapper(wasm_exec_env_t exec_env, const char* message){
        ocall_sgx_wamr_example_native_symbol(message);
    }
    static void faasmGetCurrentIdx(wasm_exec_env_t exec_env){
        if(exec_env->module_inst->module_type == Wasm_Module_Bytecode){
            WASMModuleInstance* module_instance = (WASMModuleInstance*) exec_env->module_inst;
            for(uint32_t i = 0; i < module_instance->export_func_count; i++){
                os_printf("%s\n",module_instance->export_functions[i].name);
            }
        }else {
            //AOT currently not supported, abort
            __asm("ud2");
        }
    }
    static NativeSymbol sgx_wamr_native_symbols[] = {
            {"sgx_wamr_example_native_symbol",(void*)sgx_wamr_example_native_symbol_wrapper,"($)",0x0},
            {"faasmGetCurrentIdx",(void*)faasmGetCurrentIdx,"",0x0}
    };
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr){
        *native_symbol_ptr = sgx_wamr_native_symbols;
        return sizeof(sgx_wamr_native_symbols)/sizeof(NativeSymbol);
    }
};
