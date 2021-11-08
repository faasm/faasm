#pragma once

#include <sgx/error.h>

#include <sgx_urts.h>

// This file defines the set of functions that can be called to access an SGX
// enclave from the (untrusted) Faasm runtime. ECalls in SGX terminology.
extern "C"
{
    extern faasm_sgx_status_t enclaveGetSgxSupport(void);

    extern sgx_status_t enclaveInitWamr(sgx_enclave_id_t enclaveId,
                                        faasm_sgx_status_t* returnValue);

    extern sgx_status_t enclaveLoadModule(sgx_enclave_id_t enclaveId,
                                          faasm_sgx_status_t* returnValue,
                                          const char* funcStr,
                                          const void* wasmOpCodePtr,
                                          uint32_t wasmOpCodeSize);

    extern sgx_status_t enclaveCallFunction(sgx_enclave_id_t enclaveId,
                                            faasm_sgx_status_t* returnValue,
                                            const char* funcStr);

    extern sgx_status_t enclaveTearDown(sgx_enclave_id_t enclaveId,
                                        faasm_sgx_status_t* returnValue);

    // ----- ECalls for testing purposes ----

    extern sgx_status_t enclaveCryptoChecks(sgx_enclave_id_t enclaveId,
                                            faasm_sgx_status_t* returnValue);

    extern sgx_status_t enclaveModuleStoreChecks(
      sgx_enclave_id_t enclaveId,
      faasm_sgx_status_t* returnValue);
}
