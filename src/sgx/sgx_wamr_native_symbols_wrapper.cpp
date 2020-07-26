//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/sgx_wamr_native_symbols_wrapper.h>
#include <iwasm/common/wasm_exec_env.h>
#include <iwasm/common/wasm_runtime_common.h>
#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_enclave_types.h>
extern "C"{
    extern _sgx_wamr_tcs_t* sgx_wamr_tcs;
    extern __thread uint32_t tls_thread_id;
};
#endif
extern "C"{
    extern int os_printf(const char* message, ...);
    static unsigned int faasm_get_input_size_wrapper(wasm_exec_env_t exec_env){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_get_input_size(&ret_val);
        return ret_val;
    }
    static void faasm_get_input_wrapper(wasm_exec_env_t exec_env, uint8_t* buffer, unsigned int buffer_size){
        sgx_status_t sgx_ret_val;
        ocall_faasm_get_input(buffer, buffer_size);
        return;
    }
    static void faasm_set_output_wrapper(wasm_exec_env_t exec_env, uint8_t* output, unsigned int output_size){
        sgx_status_t sgx_ret_val;
        ocall_faasm_set_output(output, output_size);
        return;
    }
    static unsigned int faasm_chain_function_input_wrapper(wasm_exec_env_t exec_env, const char *name, const uint8_t* input, unsigned int input_size){
        //Todo: sgx_ret_val
        os_printf("PTR: %p\n", sgx_wamr_tcs[tls_thread_id].response_ptr);
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_chain_function_input(&ret_val, name, input, input_size);
        return ret_val;
    }
    static unsigned int faasm_chain_this_input_wrapper(wasm_exec_env_t exec_env, int idx, const uint8_t *input_data, unsigned int input_size){
        unsigned int ret_val;
        //Todo: Shield input_data_addr and input_size.
        ocall_faasm_chain_this_input(&ret_val,idx,input_data,input_size);
        return ret_val;
    }
    static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env, unsigned int call_id){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_await_call(&ret_val, call_id);
        return ret_val;
    }
    static unsigned int faasm_await_call_output_wrapper(wasm_exec_env_t exec_env, unsigned int call_id, uint8_t* buffer, unsigned int buffer_size){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        ocall_faasm_await_call_output(&ret_val, call_id, buffer, buffer_size);
        return ret_val;
    }
    static NativeSymbol sgx_wamr_native_symbols[] = {
            {"faasmGetInputSize",(void*)faasm_get_input_size_wrapper,"()i",0x0},
            {"faasmGetInput",(void*)faasm_get_input_wrapper,"(*~)",0x0},
            {"faasmSetOutput",(void*)faasm_set_output_wrapper,"(*~)",0x0},
            {"faasmChainFunctionInput",(void*)faasm_chain_function_input_wrapper,"($*~)i",0x0},
            {"faasmChainThisInput",(void*)faasm_chain_this_input_wrapper,"(i*~)i",0x0},
            {"faasmAwaitCall",(void*)faasm_await_call_wrapper,"(i)i"},
            {"faasmAwaitCallOutput",(void*)faasm_await_call_output_wrapper,"(i*~)i",0x0}
    };
    uint32_t get_sgx_wamr_native_symbols(NativeSymbol** native_symbol_ptr){
        *native_symbol_ptr = sgx_wamr_native_symbols;
        return sizeof(sgx_wamr_native_symbols)/sizeof(NativeSymbol);
    }
};
