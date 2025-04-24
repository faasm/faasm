#include <catch2/catch.hpp>
#include <fstream>

#include "faasm_fixtures.h"

#include <enclave/outside/attestation/AttestationServiceClient.h>

namespace tests {
// 13/04/2025: TODO: this test has stopped working as we have moved to
// a self-hosted attestation service

/*
 * Fixture to set up an Azure Attestation Service client for each test.
 */
class AASClientTestFixture
{
  public:
    AASClientTestFixture()
      : client("https://localhost:8443"){};

  protected:
    sgx::AttestationServiceClient client;
    std::string correctQuoteFilePath =
      "./tests/test/attestation/files/example_correct_sgx_quote.json";
    std::string rogueQuoteFilePath =
      "./tests/test/attestation/files/example_rogue_sgx_quote.json";
    std::string rogueJwtPath =
      "./tests/test/attestation/files/example_rogue_jwt.json";
};

TEST_CASE_METHOD(AASClientTestFixture,
                 "Test obtaining JWT from enclave quote",
                 "[attestation][.]")
{
    std::string quoteFilePath;
    bool expectedSuccess;

    SECTION("Valid quote")
    {
        quoteFilePath = correctQuoteFilePath;
        expectedSuccess = true;
    }

    SECTION("Rogue quote")
    {
        quoteFilePath = rogueQuoteFilePath;
        expectedSuccess = false;
    }

    sgx::EnclaveInfo enclaveInfo(quoteFilePath);

    if (expectedSuccess) {
        std::string jwtResponse = client.attestEnclave(enclaveInfo);
    } else {
        REQUIRE_THROWS(client.attestEnclave(enclaveInfo));
    }
}

TEST_CASE_METHOD(AASClientTestFixture,
                 "Test validating JWT from Azure Attestation Service",
                 "[attestation][.]")
{
    std::string jwtResponse;
    // bool expectedSuccess;

    // Re-generate the JWT everytime in case it expires
    SECTION("Correct JWT token")
    {
        sgx::EnclaveInfo enclaveInfo(correctQuoteFilePath);
        jwtResponse = client.attestEnclave(enclaveInfo);
        // expectedSuccess = true;
    }

    SECTION("Rogue JWT token")
    {
        std::ifstream jwtFile(rogueJwtPath);
        jwtFile >> jwtResponse;
        // expectedSuccess = false;
    }

    /*
    if (expectedSuccess) {
        REQUIRE_NOTHROW(client.validateJwtToken(jwtResponse));
    } else {
        REQUIRE_THROWS(client.validateJwtToken(jwtResponse));
    }
    */
}
}
