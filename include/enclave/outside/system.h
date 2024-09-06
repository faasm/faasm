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
// TODO: probably re-visit this and have one enclave per faaslet, but support
// re-using them and pre-warming them!
#define ENCLAVE_ISOLATION_MODE_GLOBAL "global"
#define ENCLAVE_ISOLATION_MODE_FAASLET "faaslet"

namespace sgx {
void processECallErrors(
  std::string errorMessage,
  sgx_status_t sgxReturnValue,
  faasm_sgx_status_t faasmReturnValue = FAASM_SGX_SUCCESS);

std::string sgxErrorString(sgx_status_t status);

std::string faasmSgxErrorString(faasm_sgx_status_t status);

void checkSgxSetup();

// ----- Enclave management -----

sgx_enclave_id_t createEnclave();

void destroyEnclave(sgx_enclave_id_t enclaveId);

// ------ Test functions -----

void checkSgxCrypto(sgx_enclave_id_t enclaveId);
}
