#pragma once

#include <sgx/error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

extern "C"
{
    extern sgx_status_t faasm_sgx_enclave_crypto_checks(
      sgx_enclave_id_t enclave_id,
      faasm_sgx_status_t* ret_val);
}

namespace sgx {
std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxCrypto();
}
