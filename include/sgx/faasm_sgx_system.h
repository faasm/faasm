#pragma once

#include "faasm_sgx_error.h"

//SGX SDK includes
#include <sgx_eid.h>
#include <sgx_error.h>

namespace sgx {
sgx_enclave_id_t getGlobalEnclaveId();

std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxSetup();

void tearDownEnclave();
}
