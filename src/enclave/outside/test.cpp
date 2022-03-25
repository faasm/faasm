#include <faabric/util/logging.h>

#include <dlfcn.h>
#include <stdio.h>

#include <sgx_dcap_ql_wrapper.h>
#include <sgx_ql_lib_common.h>
#include <sgx_report.h>

int generateQuote()
{
    sgx_target_info_t quotingEnclaveTargetInfo;
    quote3_error_t retCode = sgx_qe_get_target_info(&quotingEnclaveTargetInfo);
    if (retCode != SGX_QL_SUCCESS) {
        printf("Error getting the quoting enclave's info: %i\n", retCode);
        return 1;
    } else {
        printf("Success in step 1!\n");
        // SPDLOG_INFO("Succes in step 1!");
    }

    return 0;
}

int main()
{
    generateQuote();

    return 0;
}
