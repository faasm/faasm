#pragma once

#include <enclave/error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

namespace sgx {
sgx_enclave_id_t getGlobalEnclaveId();

void processECallErrors(
  std::string errorMessage,
  sgx_status_t sgxReturnValue,
  faasm_sgx_status_t faasmReturnValue = FAASM_SGX_SUCCESS);

std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxSetup();

void checkSgxCrypto();

void tearDownEnclave();
}
