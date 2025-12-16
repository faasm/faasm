#pragma once

#include <string>
#include <vector>

#include <sgx_report.h>

namespace sgx {

/*
 * Class to collect information regarding an enclave that is being attested.
 * Herein we combine the enclave's EREPORT, with the quoting enclave's QUOTE,
 * and the additional enclave held data for attestation.
 */
class EnclaveInfo
{
  private:
    // Runtime-specific data for attribute-based encryption.
    std::string workflowId;
    std::string nodeId;

    // Fields derived from SGX quote.
    int enclaveType;
    std::string mrEnclaveHex;
    std::string mrSignerHex;
    std::string productIdHex;
    uint32_t securityVersion;
    uint64_t attributes;

    // Additional data for attestation.
    std::vector<uint8_t> quote;
    std::vector<uint8_t> enclaveHeldData;

  public:
    EnclaveInfo(const std::string& workflowId,
                const std::string& nodeId,
                const sgx_report_t& enclaveReport,
                const std::vector<uint8_t>& quoteBuffer,
                const std::vector<uint8_t>& enclaveHeldDataIn);

    EnclaveInfo(const std::string& jsonPath);

    const std::string& getWorkflowId() const { return workflowId; }

    const std::string& getNodeId() const { return nodeId; }

    const std::vector<uint8_t>& getQuote() const;

    const std::vector<uint8_t>& getEnclaveHeldData() const;

    void toJson(const std::string& jsonPath);
};
}
