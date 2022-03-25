#include <enclave/error.h>

#include <sgx_report.h>
#include <sgx_trts.h>
#include <sgx_utils.h>

extern "C"
{
    faasm_sgx_status_t ecallCreateReport(const sgx_target_info_t* qeTarget,
                                         const sgx_report_data_t* heldData,
                                         sgx_report_t* report)
    {
        sgx_status_t sgxReturnValue =
          sgx_create_report(qeTarget, heldData, report);

        switch (sgxReturnValue) {
            case SGX_ERROR_INVALID_PARAMETER:
                return FAASM_SGX_INVALID_PTR;
            case SGX_SUCCESS:
                return FAASM_SGX_SUCCESS;
            default:
                return FAASM_SGX_GENERATE_REPORT_FAILED;
        }
    }
}
