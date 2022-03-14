#pragma once

#include <string>
#include <vector>

namespace sgx {

// Attests that the enclave in the enclave path has the expected measure
// corresponding to the current code version. It performs the check using the
// given attestation provider URL.
bool attestEnclave(std::string attestationProviderUrl,
                   bool includeDetails,
                   std::string enclavePath,
                   std::vector<uint8_t> enclaveHeldData);
}
