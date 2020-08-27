#pragma once

#define SGX_WAMR_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"

namespace isolation {
    void checkSgxSetup(const std::string &enclavePath = SGX_WAMR_ENCLAVE_PATH,
                       int threadNumber = 0);

    void tearDownEnclave();
}