#include <enclave/outside/attestation.h>
#include <enclave/outside/ecalls.h>
#include <faabric/util/logging.h>

#include <dlfcn.h>
#include <openssl/sha.h>
#include <stdio.h>

#include <sgx_dcap_ql_wrapper.h>
#include <sgx_ql_lib_common.h>
#include <sgx_report.h>
#include <sgx_urts.h>

namespace sgx {

static void sha256sum(const uint8_t* data, uint32_t data_size, uint8_t* hash)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, data_size);
    SHA256_Final(hash, &sha256);
}

EnclaveInfo generateQuote(int enclaveId)
{
    SPDLOG_INFO("Step 0: call dlopen");
    void* handle = dlopen("libsgx_quote_ex.so.1", RTLD_LAZY | RTLD_GLOBAL);
    if (handle == nullptr) {
        SPDLOG_ERROR("Error running dlopen: {}", dlerror());
        throw std::runtime_error("Error in step 0");
    }
    SPDLOG_INFO("Success in step 0");

    SPDLOG_INFO("Step 1: call sgx_qe_get_target_info");
    sgx_target_info_t quotingEnclaveTargetInfo;
    quote3_error_t qeReturnValue =
      sgx_qe_get_target_info(&quotingEnclaveTargetInfo);
    if (qeReturnValue != SGX_QL_SUCCESS) {
        printf("Error getting the quoting enclave's info: %x\n", qeReturnValue);
        throw std::runtime_error("Error in step 1");
    }
    SPDLOG_INFO("Success in step 1");

    SPDLOG_INFO("Step 2: call create_app_report");
    std::vector<uint8_t> enclaveHeldData = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06
    };
    sgx_report_data_t enclaveHeldDataHashed;
    sha256sum(enclaveHeldData.data(), 6, enclaveHeldDataHashed.d);
    sgx_report_t enclaveReport;
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = ecallCreateReport(enclaveId,
                                                    &returnValue,
                                                    &quotingEnclaveTargetInfo,
                                                    &enclaveHeldDataHashed,
                                                    &enclaveReport);
    if (sgxReturnValue != SGX_SUCCESS) {
        // TODO - make processEcallErrors a macro usable here as well
        printf("error: %x\n", sgxReturnValue);
        throw std::runtime_error("Error in step 2");
    }
    SPDLOG_INFO("Success in step 2");

    SPDLOG_INFO("Step 3: call sgx_qe_get_quote_size");
    uint32_t quoteSize = 0;
    qeReturnValue = sgx_qe_get_quote_size(&quoteSize);
    if (qeReturnValue != SGX_QL_SUCCESS) {
        printf("Error in sgx_qe_get_quote_size. 0x%04x\n", qeReturnValue);
        throw std::runtime_error("Error in step 3");
    }
    SPDLOG_INFO("Success in step 3");

    std::vector<uint8_t> quoteBuffer(quoteSize, 0);
    SPDLOG_INFO("Step 4: call sgx_qe_get_quote");
    qeReturnValue =
      sgx_qe_get_quote(&enclaveReport, quoteSize, quoteBuffer.data());
    if (qeReturnValue != SGX_QL_SUCCESS) {
        printf("Error in sgx_qe_get_quote. 0x%04x\n", qeReturnValue);
        throw std::runtime_error("Error in step 3");
    }
    SPDLOG_INFO("Success in step 4");

    EnclaveInfo enclaveInfo(enclaveReport, quoteBuffer, enclaveHeldData);
    return enclaveInfo;
}
}
