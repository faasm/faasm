#include <enclave/outside/attestation.h>
#include <enclave/outside/attestation/AzureAttestationServiceClient.h>
#include <faabric/util/logging.h>

namespace sgx {

void validateQuote(const EnclaveInfo& enclaveInfo,
                   const std::string& attestationProviderUrl,
                   bool includeDetails)
{
    AzureAttestationServiceClient client(attestationProviderUrl);

    // TODO - enclave held data not working
    // Send enclave quote to remote attestation service for validation
    std::string jwtResponse = client.attestEnclave(enclaveInfo);

    // Validate JWT response token
    client.validateJwtToken(jwtResponse);
}
}
