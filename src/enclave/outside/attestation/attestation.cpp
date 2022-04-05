#include <conf/FaasmConfig.h>
#include <enclave/outside/attestation/attestation.h>

namespace sgx {

void attestEnclave(int enclaveId, std::vector<uint8_t> enclaveHeldData)
{
    EnclaveInfo enclaveInfo = generateQuote(enclaveId, enclaveHeldData);
    validateQuote(enclaveInfo, conf::getFaasmConfig().attestationProviderUrl);
}
}
