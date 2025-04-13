#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <enclave/outside/attestation/attestation.h>

namespace tests {
// 13/04/2025: TODO: this test has stopped working as we have moved to
// a self-hosted attestation service
TEST_CASE_METHOD(FaasmConfTestFixture,
                 "Test SGX quote validation",
                 "[attestation][.]")
{
    faasmConf.attestationServiceUrl = "https://localhost:8443";
    std::string quoteFilePath;
    bool expectedSuccess;

    SECTION("Valid quote")
    {
        quoteFilePath =
          "./tests/test/attestation/files/example_correct_sgx_quote.json";
        expectedSuccess = true;
    }

    SECTION("Invalid quote")
    {
        quoteFilePath =
          "./tests/test/attestation/files/example_rogue_sgx_quote.json";
        expectedSuccess = false;
    }

    sgx::EnclaveInfo enclaveInfo(quoteFilePath);

    if (expectedSuccess) {
        REQUIRE_NOTHROW(
          sgx::validateQuote(enclaveInfo, faasmConf.attestationServiceUrl));
    } else {
        REQUIRE_THROWS(
          sgx::validateQuote(enclaveInfo, faasmConf.attestationServiceUrl));
    }
}
}
