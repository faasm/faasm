#ifndef SGX_RUNNER_ENCLAVE_PATH
#define SGX_RUNNER_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"
#endif

#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/SGXWAMRWasmModule.h>

#include <faabric/util/func.h>
#include <cstdio>


int main(int argc, char **argv) {
    // Set up the module
    sgx_enclave_id_t enclaveId;
    wasm::SGXWAMRWasmModule module(&enclaveId);

    if (argc < 3) {
        printf("[Error] Too few arguments. Please enter user and function\n");
        return -1;
    }

    // Check for SGX support
    faasm_sgx_status_t returnValue;
#if(SGX_SIM_MODE == 0)
    returnValue = faasm_sgx_get_sgx_support();
    if (returnValue != FAASM_SGX_SUCCESS) {
        printf("[Error] Missing sgx support (%#010x)\n", returnValue);
        return -1;
    }
#endif

    // Set up the enclave
    sgx_status_t sgxReturnValue;
    sgx_launch_token_t enclaveToken = {0};
    int enclaveTokenUpdated = 0;
    if (argc > 3) {
        sgxReturnValue = sgx_create_enclave(
                argv[3],
                SGX_DEBUG_FLAG,
                &enclaveToken,
                &enclaveTokenUpdated,
                &enclaveId,
                NULL
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to create enclave (%#010x)\n", sgxReturnValue);
            return -1;
        }
    } else {
        sgxReturnValue = sgx_create_enclave(
                SGX_RUNNER_ENCLAVE_PATH,
                SGX_DEBUG_FLAG,
                &enclaveToken,
                &enclaveTokenUpdated,
                &enclaveId,
                NULL
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to create enclave (%#010x)\n", sgxReturnValue);
            return -1;
        }
    }

    // Set up WAMR
    sgxReturnValue = sgx_wamr_enclave_init_wamr(enclaveId, &returnValue, 1);
    if (sgxReturnValue != SGX_SUCCESS) {
        printf("[Error] Unable to enter enclave (%#010x)\n", sgxReturnValue);
        return -1;
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        printf("[Error] Unable to initialize WAMR (%#010x)\n", returnValue);
        return -1;
    }

    // Execute the function
    faabric::Message msg = faabric::util::messageFactory(argv[1], argv[2]);
    msg.set_issgx(true);
    module.bindToFunction(msg);
    module.execute(msg);
}