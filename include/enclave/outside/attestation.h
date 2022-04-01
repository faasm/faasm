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
    int enclaveType;
    std::string mrEnclaveHex;
    std::string mrSignerHex;
    std::string productIdHex;
    uint32_t securityVersion;
    uint64_t attributes;
    std::string quoteHex;
    std::string enclaveHeldDatHex;

  public:
    EnclaveInfo(const sgx_report_t& enclaveReport,
                const std::vector<uint8_t>& quoteBuffer,
                const std::vector<uint8_t>& enclaveHeldData);
};

// Generate a quote for an enclave. A quote is a signed report of all the
// contents of the enclave's memory.
EnclaveInfo generateQuote(int enclaveId);

// Validate an enclave's quote through an external attestation service.
// TODO - the attest DNS should be an environmental variable
bool validateQuote(const EnclaveInfo& enclaveInfo,
                   std::string attestDnsName,
                   bool includeDetails);

// Attests that the enclave in the enclave path has the expected measure
// corresponding to the current code version. It performs the check using the
// given attestation provider URL.
bool attestEnclave(std::string attestationProviderUrl,
                   bool includeDetails,
                   std::string enclavePath,
                   std::vector<uint8_t> enclaveHeldData);
}
