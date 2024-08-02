#include <enclave/error.h>

#include <sgx.h>
#include <sgx_report.h>
#include <sgx_urts.h>

// What follows are the definitions of the enclave-entry calls. This is, the
// way we have to interact with a running enclave. These calls are shared among
// all function invocations (i.e. module instantiation).
extern "C"
{
    extern sgx_status_t ecallCreateReport(sgx_enclave_id_t enclaveId,
                                          faasm_sgx_status_t* retVal,
                                          const sgx_target_info_t* qeTarget,
                                          const sgx_report_data_t* heldData,
                                          sgx_report_t* report);

    extern sgx_status_t ecallInitWamr(sgx_enclave_id_t enclaveId,
                                      faasm_sgx_status_t* retVal);

    extern sgx_status_t ecallReset(sgx_enclave_id_t enclaveId,
                                   faasm_sgx_status_t* retVal);

    extern sgx_status_t ecallDoBindToFunction(sgx_enclave_id_t enclaveId,
                                              faasm_sgx_status_t* retVal,
                                              const char* user,
                                              const char* func,
                                              const void* wasmOpCodePtr,
                                              const uint32_t wasmOpCodeSize,
                                              uint32_t faasletId);

    extern sgx_status_t ecallDestroyModule(sgx_enclave_id_t enclaveId,
                                           faasm_sgx_status_t* retVal,
                                           uint32_t faasletId);

    extern sgx_status_t ecallCallFunction(sgx_enclave_id_t enclaveId,
                                          faasm_sgx_status_t* retVal,
                                          uint32_t faasletId,
                                          uint32_t argc,
                                          char** argv);

    extern sgx_status_t ecallCryptoChecks(sgx_enclave_id_t enclaveId,
                                          faasm_sgx_status_t* retVal);
}
