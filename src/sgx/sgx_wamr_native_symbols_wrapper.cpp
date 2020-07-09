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
    static unsigned int faasm_chain_function_input(wasm_exec_env_t exec_env, const char *name, const uint8_t* input, long input_size){
        //Todo: sgx_ret_val
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_chain_function_input(&ret_val, name, input, input_size);
        return ret_val;
    }
    static unsigned int faasm_chain_this_input_wrapper(wasm_exec_env_t exec_env, int idx, const uint8_t *input_data, long input_size){
        unsigned int ret_val;
        //Todo: Shield input_data_addr and input_size.
        ocall_faasm_chain_this_input(&ret_val,idx,input_data,input_size);
        return ret_val;
    }
    static unsigned int faasm_await_call(wasm_exec_env_t exec_env, unsigned int call_id){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_await_call(&ret_val, call_id);
        return ret_val;
    }
    static NativeSymbol sgx_wamr_native_symbols[] = {
            {"faasmGetCurrentIdx",(void*)faasmGetCurrentIdx,"",0x0},
            {"faasmChainFunctionInput",(void*)faasm_chain_function_input,"($*~)i",0x0},
            {"faasmChainThisInput",(void*)faasm_chain_this_input_wrapper,"(i*~)i",0x0},
            {"faasmAwaitCall",(void*)faasm_await_call,"(i)i"}
    };
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr){
        *native_symbol_ptr = sgx_wamr_native_symbols;
        return sizeof(sgx_wamr_native_symbols)/sizeof(NativeSymbol);
    }
};
