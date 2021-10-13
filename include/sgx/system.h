#pragma once

#include <sgx/ecalls.h>
#include <sgx/error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

namespace sgx {
std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxCrypto();
}
