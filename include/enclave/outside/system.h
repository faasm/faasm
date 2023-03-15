#pragma once

#include <enclave/error.h>

#include <sgx_eid.h>
#include <sgx_error.h>
#include <string>

// We support two different isolation modes in Faasm for Faaslets running
// inside SGX enclaves. Global isolation means that we only ever create one
// enclave per runtime instance, and all Faaslets share the same enclave.
// Faaslet isolation means that we create a different enclave for each Faaslet
// and destroy it at the end.
#define ENCLAVE_ISOLATION_MODE_GLOBAL "global"
#define ENCLAVE_ISOLATION_MODE_FAASLET "faaslet"

namespace sgx {
sgx_enclave_id_t getGlobalEnclaveId();

void processECallErrors(
  std::string errorMessage,
  sgx_status_t sgxReturnValue,
  faasm_sgx_status_t faasmReturnValue = FAASM_SGX_SUCCESS);

std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

sgx_enclave_id_t checkSgxSetup();

void checkSgxCrypto();

void tearDownEnclave();
}
