#pragma once

#include <sgx/error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

#define SGX_MODULE_STORE_SIZE 32
#define SGX_MODULE_STORE_UNSET (SGX_MODULE_STORE_SIZE + 1)

namespace sgx {
std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxCrypto();
}
