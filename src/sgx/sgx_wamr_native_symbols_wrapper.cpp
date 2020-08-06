//
// Created by Joshua Heinemann on 17.06.20.
// TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
//

#include <sgx/faasm_sgx_error.h>
#include <sgx/sgx_wamr_native_symbols_wrapper.h>
#include <iwasm/interpreter/wasm_runtime.h>
#include <iwasm/common/wasm_exec_env.h>
#if(FAASM_SGX_ATTESTATION)
#include <sgx/sgx_wamr_enclave_types.h>
extern "C"{
    extern _sgx_wamr_tcs_t* sgx_wamr_tcs;
    extern __thread uint32_t tls_thread_id;
};
#endif
#define SET_ERROR(X)\
memcpy(((WASMModuleInstance*)exec_env->module_inst)->cur_exception,_WRAPPER_ERROR_PREFIX,sizeof(_WRAPPER_ERROR_PREFIX));\
*((uint32_t*)&((WASMModuleInstance*)exec_env->module_inst)->cur_exception[sizeof(_WRAPPER_ERROR_PREFIX)]) = (X);
extern "C"{
    extern int os_printf(const char* message, ...);
    static uint64_t faasm_read_state_size_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        uint64_t ret_val;
        if((sgx_ret_val = ocall_faasm_read_state_size(&ret_val,key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static uint64_t faasm_read_state_wrapper(wasm_exec_env_t exec_env, const char* key, uint8_t* buffer_ptr, const uint32_t buffer_len){
        sgx_status_t sgx_ret_val;
        uint64_t ret_val;
        if((sgx_ret_val = ocall_faasm_read_state(&ret_val, key, buffer_ptr, buffer_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static void faasm_read_appended_state_wrapper(wasm_exec_env_t exec_env, const char* key, uint8_t* buffer_ptr, const uint32_t buffer_len, const uint32_t element_num){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_read_appended_state(key, buffer_ptr, buffer_len, element_num)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_read_state_offset_wrapper(wasm_exec_env_t exec_env, const char* key, const uint64_t total_len, const uint64_t offset, uint8_t* buffer_ptr, const uint32_t buffer_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_read_state_offset(key,total_len,offset,buffer_ptr,buffer_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_write_state_wrapper(wasm_exec_env_t exec_env, const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_write_state(key, buffer_ptr, buffer_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_append_state_wrapper(wasm_exec_env_t exec_env, const char* key, const uint8_t* buffer_ptr, const uint32_t buffer_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_append_state(key,buffer_ptr, buffer_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_clear_appended_state_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_clear_appended_state(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_write_state_offset_wrapper(wasm_exec_env_t exec_env, const char* key, const uint64_t total_len, const uint64_t offset, const uint8_t* buffer_ptr, const uint32_t buffer_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_write_state_offset(key,total_len,offset,buffer_ptr,buffer_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_flag_state_dirty_wrapper(wasm_exec_env_t exec_env, const char* key, const uint64_t total_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_flag_state_dirty(key,total_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_flag_state_offset_dirty_wrapper(wasm_exec_env_t exec_env, const char* key, const uint64_t total_len, const uint64_t offset, const uint64_t len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_flag_state_offset_dirty(key,total_len,offset,len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_push_state_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_push_state(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_push_state_partial_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_push_state_partial(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_push_state_partial_mask_wrapper(wasm_exec_env_t exec_env, const char* key, const char* mask_key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_push_state_partial_mask(key,mask_key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_pull_state_wrapper(wasm_exec_env_t exec_env, const char* key, const uint64_t state_len){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_pull_state(key,state_len)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_lock_state_global_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_lock_state_global(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_unlock_state_global_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_unlock_state_global(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_lock_state_read_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_lock_state_read(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_unlock_state_read_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_unlock_state_read(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_lock_state_write_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_lock_state_write(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_unlock_state_write_wrapper(wasm_exec_env_t exec_env, const char* key){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_unlock_state_write(key)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static unsigned int faasm_get_input_size_wrapper(wasm_exec_env_t exec_env){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        if((sgx_ret_val = ocall_faasm_get_input_size(&ret_val)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static void faasm_get_input_wrapper(wasm_exec_env_t exec_env, uint8_t* buffer, unsigned int buffer_size){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_get_input(buffer, buffer_size)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static void faasm_set_output_wrapper(wasm_exec_env_t exec_env, uint8_t* output, unsigned int output_size){
        sgx_status_t sgx_ret_val;
        if((sgx_ret_val = ocall_faasm_set_output(output, output_size)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return;
    }
    static unsigned int faasm_chain_function_input_wrapper(wasm_exec_env_t exec_env, const char *name, const uint8_t* input, unsigned int input_size){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        if((sgx_ret_val = ocall_faasm_chain_function_input(&ret_val, name, input, input_size)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static unsigned int faasm_chain_this_input_wrapper(wasm_exec_env_t exec_env, int idx, const uint8_t *input_data, unsigned int input_size){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        if((sgx_ret_val = ocall_faasm_chain_this_input(&ret_val,idx,input_data,input_size)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static unsigned int faasm_await_call_wrapper(wasm_exec_env_t exec_env, unsigned int call_id){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        if((sgx_ret_val = ocall_faasm_await_call(&ret_val, call_id)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static unsigned int faasm_await_call_output_wrapper(wasm_exec_env_t exec_env, unsigned int call_id, uint8_t* buffer, unsigned int buffer_size){
        sgx_status_t sgx_ret_val;
        unsigned int ret_val;
        if((sgx_ret_val = ocall_faasm_await_call_output(&ret_val, call_id, buffer, buffer_size)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
    static int faasm_get_current_idx_wrapper(wasm_exec_env_t exec_env){
        sgx_status_t sgx_ret_val;
        int ret_val;
        if((sgx_ret_val = ocall_faasm_get_current_idx(&ret_val)) != SGX_SUCCESS){
            SET_ERROR(FAASM_SGX_OCALL_ERROR(sgx_ret_val));
        }
        return ret_val;
    }
#if(FAASM_SGX_WHITELISTING)
    void sgx_wamr_function_not_whitelisted_wrapper(wasm_exec_env_t exec_env){
        SET_ERROR(FAASM_SGX_FUNCTION_NOT_WHITELISTED);
        return;
    }
#endif
NativeSymbol sgx_wamr_native_symbols[28] = {
            {"faasmReadStateSize",(void*)faasm_read_state_size_wrapper,"($)I",0x0},
            {"faasmReadState",(void*)faasm_read_state_wrapper,"($*~)I",0x0},
            {"faasmReadAppendedState",(void*)faasm_read_appended_state_wrapper,"($*~i)",0x0},
            {"faasmReadStateOffset",(void*)faasm_read_state_offset_wrapper,"($II*~)",0x0},
            {"faasmWriteState",(void*)faasm_write_state_wrapper,"($*~)",0x0},
            {"faasmAppendState",(void*)faasm_append_state_wrapper,"($*~)",0x0},
            {"faasmClearAppendedState",(void*)faasm_clear_appended_state_wrapper,"($)",0x0},
            {"faasmWriteStateOffset",(void*)faasm_write_state_offset_wrapper,"($II*~)",0x0},
            {"faasmFlagStateDirty",(void*)faasm_flag_state_dirty_wrapper,"($I)",0x0},
            {"faasmFlagStateOffsetDirty",(void*)faasm_flag_state_offset_dirty_wrapper,"($III)",0x0},
            {"faasmPushState",(void*)faasm_push_state_wrapper,"($)",0x0},
            {"faasmPushStatePartial",(void*)faasm_push_state_partial_wrapper,"($)",0x0},
            {"faasmPushStatePartialMask",(void*)faasm_push_state_partial_mask_wrapper,"($$)",0x0},
            {"faasmPullState",(void*)faasm_pull_state_wrapper,"($I)"},
            {"faasmLockStateGlobal",(void*)faasm_lock_state_global_wrapper,"($)",0x0},
            {"faasmUnlockStateGlobal",(void*)faasm_unlock_state_global_wrapper,"($)",0x0},
            {"faasmLockStateRead",(void*)faasm_lock_state_read_wrapper,"($)",0x0},
            {"faasmUnlockStateRead",(void*)faasm_unlock_state_read_wrapper,"($)",0x0},
            {"faasmLockStateWrite",(void*)faasm_lock_state_write_wrapper,"($)",0x0},
            {"faasmUnlockStateWrite",(void*)faasm_unlock_state_write_wrapper,"($)",0x0},
            {"faasmGetInputSize",(void*)faasm_get_input_size_wrapper,"()i",0x0},
            {"faasmGetInput",(void*)faasm_get_input_wrapper,"(*~)",0x0},
            {"faasmSetOutput",(void*)faasm_set_output_wrapper,"(*~)",0x0},
            {"faasmChainFunctionInput",(void*)faasm_chain_function_input_wrapper,"($*~)i",0x0},
            {"faasmChainThisInput",(void*)faasm_chain_this_input_wrapper,"(i*~)i",0x0},
            {"faasmAwaitCall",(void*)faasm_await_call_wrapper,"(i)i",0x0},
            {"faasmAwaitCallOutput",(void*)faasm_await_call_output_wrapper,"(i*~)i",0x0},
            {"faasmGetCurrentIdx",(void*)faasm_get_current_idx_wrapper,"()i",0x0}
    };
};
