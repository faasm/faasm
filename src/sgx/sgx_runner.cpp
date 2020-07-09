
#ifndef SGX_RUNNER_ENCLAVE_PATH
#define SGX_RUNNER_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"
#endif

#include <stdio.h>
#include <sgx.h>
#include <sgx_urts.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <util/func.h>

int main(int argc, char **argv) {
    sgx_launch_token_t enclave_token = {0};
    sgx_status_t sgx_ret_val;
    faasm_sgx_status_t ret_val;
    int enclave_token_updated = 0;

    sgx_enclave_id_t enclave_id;
    wasm::SGXWAMRWasmModule module(&enclave_id);

    if (argc < 3) {
        printf("[Error] Too few arguments. Please enter user and function\n");
        return -1;
    }

#if(SGX_SIM_MODE == 0)
    if ((ret_val = faasm_sgx_get_sgx_support()) != FAASM_SGX_SUCCESS) {
        printf("[Error] Missing sgx support (%#010x)\n", ret_val);
        return -1;
    }
#endif

    message::Message msg = util::messageFactory(argv[1], argv[2]);
    msg.set_issgx(true);

    if (argc > 3) {
        if ((sgx_ret_val = sgx_create_enclave(argv[3], SGX_DEBUG_FLAG, &enclave_token, &enclave_token_updated,
                                              &enclave_id, NULL)) != SGX_SUCCESS) {
            printf("[Error] Unable to create enclave (%#010x)\n", sgx_ret_val);
            return -1;
        }
    } else {
        if ((sgx_ret_val = sgx_create_enclave(SGX_RUNNER_ENCLAVE_PATH, SGX_DEBUG_FLAG, &enclave_token,
                                              &enclave_token_updated, &enclave_id, NULL)) != SGX_SUCCESS) {
            printf("[Error] Unable to create enclave (%#010x)\n", sgx_ret_val);
            return -1;
        }
    }

    if ((sgx_ret_val = sgx_wamr_enclave_init_wamr(enclave_id, &ret_val, 1)) != SGX_SUCCESS) {
        printf("[Error] Unable to enter enclave (%#010x)\n", sgx_ret_val);
        return -1;
    }
    if (ret_val != FAASM_SGX_SUCCESS) {
        printf("[Error] Unable to initialize WAMR (%#010x)\n", ret_val);
        return -1;
    }

    module.bindToFunction(msg);
    module.execute(msg);
}