#include <catch2/catch.hpp>

#include <enclave/outside/system.h>

namespace tests {
class SgxTestFixture
{
  public:
    SgxTestFixture() { enclaveId = sgx::createEnclave(); }

    ~SgxTestFixture() { sgx::destroyEnclave(enclaveId); }

  protected:
    sgx_enclave_id_t enclaveId;
};

TEST_CASE("Test enclave set up and tear down", "[.][sgx]")
{
    auto enclaveId = sgx::createEnclave();

    REQUIRE(enclaveId != 0);

    sgx::destroyEnclave(enclaveId);
}

TEST_CASE_METHOD(SgxTestFixture, "Test SGX crypto checks", "[.][sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto(enclaveId));
}
}
