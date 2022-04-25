#include <catch2/catch.hpp>
#include <cstdio>

#include <enclave/outside/attestation/EnclaveInfo.h>

namespace tests {
TEST_CASE("Test loading enclave info from file and dumping to JSON",
          "[attestation]")
{
    sgx::EnclaveInfo enclaveInfo(
      "./tests/test/attestation/files/example_correct_sgx_quote.json");

    // Test that the getters work
    auto originalQuote = enclaveInfo.getQuote();
    REQUIRE(originalQuote.size() != 0);
    auto originalEnclaveHeldData = enclaveInfo.getEnclaveHeldData();
    REQUIRE(originalEnclaveHeldData.size() != 0);

    std::string tmpJsonPath = "/tmp/sgx_quote.json";
    enclaveInfo.toJson(tmpJsonPath);
    sgx::EnclaveInfo enclaveInfoTmp(tmpJsonPath);

    REQUIRE(enclaveInfoTmp.getQuote() == originalQuote);
    REQUIRE(enclaveInfoTmp.getEnclaveHeldData() == originalEnclaveHeldData);

    // Remove tmp file
    std::remove(tmpJsonPath.c_str());
}
}
