#include <enclave/outside/attestation/ecalls.h>
#include <faabric/util/logging.h>

#include <stdio.h>
#include <dlfcn.h>
#include <openssl/sha.h>

#include <sgx_dcap_ql_wrapper.h>
#include <sgx_ql_lib_common.h>
#include <sgx_report.h>
#include <sgx_urts.h>

static void sha256sum(const uint8_t *data, uint32_t data_size, uint8_t *hash)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, data_size);
    SHA256_Final(hash, &sha256);
}

int generateQuote(int enclaveId)
{
    SPDLOG_INFO("Step 0");
    dlopen("libsgx_quote_ex.so.1", RTLD_GLOBAL);
    printf("\nStep1: Call sgx_qe_get_target_info: \n");
    SPDLOG_INFO("Step 1");
    sgx_target_info_t quotingEnclaveTargetInfo;
    quote3_error_t retCode = sgx_qe_get_target_info(&quotingEnclaveTargetInfo);
    if (retCode != SGX_QL_SUCCESS) {
        printf("Error getting the quoting enclave's info: %x\n", retCode);
        return 1;
    }
    printf("Success in step 1!\n");

    printf("\nStep2: Call create_app_report: ");
    uint8_t enclaveHeldData[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    sgx_report_data_t enclaveHeldDataHashed;
    sha256sum(enclaveHeldData, 6, enclaveHeldDataHashed.d);
    sgx_report_t enclaveReport;
    faasm_sgx_status_t returnValue;
    printf("enclave id: %i\n", enclaveId);
    sgx_status_t sgxReturnValue = ecallCreateReport(enclaveId,
                                                    &returnValue,
                                                    &quotingEnclaveTargetInfo,
                                                    &enclaveHeldDataHashed,
                                                    &enclaveReport);
    if (sgxReturnValue != SGX_SUCCESS) {
        // TODO - make processEcallErrors a macro usable here as well
        printf("error: %x\n", sgxReturnValue);
        printf("failed\n");
    }
    printf("Success in step 2!\n");
    printf("report mac: %s\n", enclaveReport.mac);

    return 0;
}
