#include <catch2/catch.hpp>

#include <sgx/faasm_sgx_system.h>

namespace tests {
class SgxTestFixture
{
  public:
    SgxTestFixture() { sgx::checkSgxSetup(); }

    ~SgxTestFixture() { sgx::tearDownEnclave(); }
};

TEST_CASE("Test enclave set up and tear down", "[sgx]")
{
    REQUIRE(sgx::getGlobalEnclaveId() <= 0);

    // Initialise the global enclave
    sgx::checkSgxSetup();

    REQUIRE(sgx::getGlobalEnclaveId() > 0);

    sgx::tearDownEnclave();
}

TEST_CASE_METHOD(SgxTestFixture, "Test SGX crypto checks", "[sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto());
}
}
