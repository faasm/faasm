#pragma once

#include <enclave/outside/attestation/EnclaveInfo.h>

#include <string>
#include <vector>

namespace sgx {

// Generate a quote for an enclave. A quote is a signed report of all the
// contents of the enclave's memory.
EnclaveInfo generateQuote(int enclaveId,
                          const std::vector<uint8_t>& enclaveHeldData);

// Validate an enclave's quote through an external attestation service.
void validateQuote(const EnclaveInfo& enclaveInfo,
                   const std::string& attestDnsName,
                   bool includeDetails);

// Attests that the enclave in the enclave path has the expected measure
// corresponding to the current code version. It performs the check using the
// given attestation provider URL.
bool attestEnclave(std::string attestationProviderUrl,
                   bool includeDetails,
                   std::string enclavePath,
                   std::vector<uint8_t> enclaveHeldData);
}
