#pragma once

#include <sgx_eid.h>
#include <sgx_error.h>
#include <faasm_sgx_error.h>

#define SGX_WAMR_ENCLAVE_PATH "sgx_wamr_enclave.sign.so"

namespace sgx {
    sgx_enclave_id_t getGlobalEnclaveId();

    std::string sgxErrorString(sgx_status_t status);

    std::string faasmSgxErrorString(faasm_sgx_status_t status);

    void checkSgxSetup(
            const std::string &enclavePath = SGX_WAMR_ENCLAVE_PATH,
            int threadNumber = 0
    );

    void tearDownEnclave();
}