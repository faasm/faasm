#include <enclave/outside/attestation.h>
#include <faabric/util/logging.h>

namespace sgx {

bool validateQuote(const EnclaveInfo& enclaveInfo,
                   std::string attestationProviderUrl,
                   bool includeDetails)
{
    SPDLOG_ERROR("validateQuote not implemented");
    return true;
}
}
