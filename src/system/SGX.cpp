
#ifndef FAASM_SGX

namespace isolation {
    void checkSgxSetup() {
        // Do nothing
    }

    void tearDownEnclave() {
        // Do nothing
    }
}

#else

#define SGX_WAMR_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"
#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/faasm_sgx_error.h>
#include <sgx/SGXWAMRWasmModule.h>

extern "C"{
sgx_enclave_id_t enclave_id;
};

namespace isolation {
    void checkSgxSetup() {
        faasm_sgx_status_t ret_val;
        sgx_status_t sgx_ret_val;
        sgx_launch_token_t sgx_enclave_token = {0};
        uint32_t sgx_enclave_token_updated = 0;

        #if(SGX_SIM_MODE == 0)
        if((ret_val = faasm_sgx_get_sgx_support()) != FAASM_SGX_SUCCESS){
            printf("[Error] Machine doesn't support sgx (%#010x)\n",ret_val);
            exit(0);
        }
        #endif

        if((sgx_ret_val = sgx_create_enclave(SGX_WAMR_ENCLAVE_PATH,SGX_DEBUG_FLAG,&sgx_enclave_token,(int*)&sgx_enclave_token_updated,&enclave_id,NULL)) != SGX_SUCCESS){
            printf("[Error] Unable to create enclave (%#010x)\n",sgx_ret_val);
            exit(0);
        }
        if((sgx_ret_val = sgx_wamr_enclave_init_wamr(enclave_id,&ret_val, 0)) != SGX_SUCCESS){
            printf("[Error] Unable to enter enclave (%#010x)\n",sgx_ret_val);
            exit(0);
        }
        if(ret_val != FAASM_SGX_SUCCESS){
            printf("[Error] Unable to initialize WAMR (%#010x)\n",ret_val);
            exit(0);
        }
    }

    void tearDownEnclave() {
        sgx_status_t sgx_ret_val;

        printf("[Info] Destroying enclave\n");
        if((sgx_ret_val = sgx_destroy_enclave(enclave_id)) != SGX_SUCCESS){
            printf("[Info] Unable to destroy enclave (%#010x)\n",sgx_ret_val);
        }
    }
}

#endif