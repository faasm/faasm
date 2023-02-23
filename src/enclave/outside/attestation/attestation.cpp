#include <conf/FaasmConfig.h>
#include <enclave/outside/attestation/AzureAttestationServiceClient.h>
#include <enclave/outside/attestation/attestation.h>
#include <enclave/outside/ecalls.h>
#include <faabric/util/logging.h>

#ifdef FAASM_SGX_HARDWARE_MODE
#include <dlfcn.h>
#include <openssl/evp.h>
#include <sgx_dcap_ql_wrapper.h>
#include <sgx_ql_lib_common.h>
#include <sgx_report.h>
#include <sgx_urts.h>
#endif

namespace sgx {

#ifdef FAASM_SGX_HARDWARE_MODE
static void sha256sum(const uint8_t* data, uint32_t data_size, uint8_t* hash)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data, data_size);
    EVP_DigestFinal_ex(ctx, hash, &data_size);
    EVP_MD_CTX_free(ctx);
}

// To generate an enclave quote we need to do four steps:
// 1. Query the state of the Quoting Enclave (QE)
// 2. Generate the enclave report
// 3. Prepare the enclave quote buffer
// 4. Send the enclave report to the QE to get the enclave quote in return
// For more information see the SGX docs and the header file where this method
// is declared.
EnclaveInfo generateQuote(int enclaveId,
                          const std::vector<uint8_t>& enclaveHeldData)
{
    // Step 1: query the state of the Quoting Enclave (QE)
    sgx_target_info_t quotingEnclaveTargetInfo;
    quote3_error_t qeReturnValue =
      sgx_qe_get_target_info(&quotingEnclaveTargetInfo);
    if (qeReturnValue != SGX_QL_SUCCESS) {
        SPDLOG_ERROR("Error getting the quoting enclave's info: 0x{:04X}",
                     qeReturnValue);
        throw std::runtime_error("Error getting the QE's info");
    }
    SPDLOG_DEBUG("Success communicating with the Quoting Enclave");

    // The enclave held data is an additional piece of information that can be
    // included as part of the attestation process
    sgx_report_data_t enclaveHeldDataHashed;
    sha256sum(
      &enclaveHeldData[0], enclaveHeldData.size(), enclaveHeldDataHashed.d);

    // Step 2: generate the enclave report
    sgx_report_t enclaveReport;
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = ecallCreateReport(enclaveId,
                                                    &returnValue,
                                                    &quotingEnclaveTargetInfo,
                                                    &enclaveHeldDataHashed,
                                                    &enclaveReport);
    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("Error creating enclave's report: 0x{:04X}",
                     qeReturnValue);
        throw std::runtime_error("Error creating enclave's report");
    }
    SPDLOG_DEBUG("Success generating enclave's report");

    // Step 3: prepare the enclave quote buffer
    uint32_t quoteSize = 0;
    qeReturnValue = sgx_qe_get_quote_size(&quoteSize);
    if (qeReturnValue != SGX_QL_SUCCESS) {
        SPDLOG_ERROR("Error getting enclave'squote size: 0x{:04X}",
                     qeReturnValue);
        throw std::runtime_error("Error getting enclave's quote size");
    }

    // Step 4: send the enclave report to the QE to get the quote in return
    std::vector<uint8_t> quoteBuffer(quoteSize, 0);
    qeReturnValue =
      sgx_qe_get_quote(&enclaveReport, quoteSize, &quoteBuffer[0]);
    if (qeReturnValue != SGX_QL_SUCCESS) {
        SPDLOG_ERROR("Error in getting enclave's quote: 0x{:04X}",
                     qeReturnValue);
        throw std::runtime_error("Error getting enclave's quote");
    }

    // Wrap the received information in a convinient wrapper
    EnclaveInfo enclaveInfo(enclaveReport, quoteBuffer, enclaveHeldData);
    SPDLOG_INFO("Generated attestation quote for enclave");
    return enclaveInfo;
}
#endif

void validateQuote(const EnclaveInfo& enclaveInfo,
                   const std::string& attestationProviderUrl)
{
    AzureAttestationServiceClient client(attestationProviderUrl);

    // Send enclave quote to remote attestation service for validation
    std::string jwtResponse = client.attestEnclave(enclaveInfo);

    // Validate JWT response token
    client.validateJwtToken(jwtResponse);
}

#ifdef FAASM_SGX_HARDWARE_MODE
void attestEnclave(int enclaveId, std::vector<uint8_t> enclaveHeldData)
{
    EnclaveInfo enclaveInfo = generateQuote(enclaveId, enclaveHeldData);
    validateQuote(enclaveInfo, conf::getFaasmConfig().attestationProviderUrl);
}
#endif
}
