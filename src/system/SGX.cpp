
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

#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/faasm_sgx_error.h>
#include <sgx/SGXWAMRWasmModule.h>

extern "C" {
sgx_enclave_id_t enclaveId;
}

namespace isolation {
    void checkSgxSetup(const std::string &enclavePath,
                       int threadNumber) {
        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue;
        sgx_launch_token_t sgxEnclaveToken = {0};
        uint32_t sgxEnclaveTokenUpdated = 0;

#if(SGX_SIM_MODE == 0)
        if((returnValue = faasm_sgx_get_sgx_support()) != FAASM_SGX_SUCCESS){
            printf("[Error] Machine doesn't support sgx (%#010x)\n",ret_val);
            exit(1);
        }
#endif

        sgxReturnValue = sgx_create_enclave(
                enclavePath.c_str(),
                SGX_DEBUG_FLAG,
                &sgxEnclaveToken,
                (int *) &sgxEnclaveTokenUpdated,
                &enclaveId,
                nullptr
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to create enclave (%#010x)\n", sgxReturnValue);
            exit(0);
        }

        sgxReturnValue = sgx_wamr_enclave_init_wamr(enclaveId, &returnValue, threadNumber);
        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgxReturnValue);
            exit(0);
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to initialize WAMR (%#010x)\n", returnValue);
            exit(0);
        }
    }

    void tearDownEnclave() {
        sgx_status_t sgx_ret_val;

        printf("[Info] Destroying enclave\n");
        if ((sgx_ret_val = sgx_destroy_enclave(enclaveId)) != SGX_SUCCESS) {
            printf("[Info] Unable to destroy enclave (%#010x)\n", sgx_ret_val);
        }
    }
}

#endif