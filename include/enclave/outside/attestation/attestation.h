#pragma once

#include <enclave/outside/attestation/EnclaveInfo.h>

#include <string>
#include <vector>

namespace sgx {

// Get QE target info
sgx_target_info_t getQuotingEnclaveTargetInfo();

// Given an SGX report, get a quote from the Quoting Enclave
std::vector<uint8_t> getQuoteFromReport(sgx_report_t report);

// An enclave report is a signed measure of the enclave's memory contents. To
// remotely attest the enclave report, it needs to be signed again by the
// Quoting Enclave (QE), a (unique) Intel-provisioned enclave running in the
// same host. An enclave quote is the enclave report signed by the QE.
EnclaveInfo generateQuote(int enclaveId,
                          const std::vector<uint8_t>& enclaveHeldData);

// Validate an enclave's quote through an external attestation service.
// Validation happens in two steps. First, we send the enclave's quote to an
// external attestation service that checks the quote against an attestation
// policy, and returns a JWT upon succesful validation. Second, we validate
// the integrity and signature of the JWT.
void validateQuote(const EnclaveInfo& enclaveInfo,
                   const std::string& attestationProviderUrl);

// Attests that the enclave identified by the unique enclave id. Remote
// attestation happens in two steps. First, we generate a quote for the
// enclave. Second, we validate the quote with an external attestation service.
void attestEnclave(int enclaveId, std::vector<uint8_t> enclaveHeldData);
}
