#pragma once

#include <sgx/faasm_sgx_error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

namespace sgx {
sgx_enclave_id_t getGlobalEnclaveId();

std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxSetup();

void tearDownEnclave();
}
