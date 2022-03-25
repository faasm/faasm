#include <enclave/error.h>

#include <sgx.h>
#include <sgx_urts.h>

extern "C"
{
    extern sgx_status_t ecallCreateReport(sgx_enclave_id_t enclaveId,
                                          faasm_sgx_status_t* retVal,
                                          const sgx_target_info_t* qeTarget,
                                          const sgx_report_data_t* heldData,
                                          sgx_report_t* report);
}
