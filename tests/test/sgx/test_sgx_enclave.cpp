#include <catch2/catch.hpp>

#include <sgx/WAMREnclave.h>
#include <sgx/system.h>

namespace tests {
class SgxTestFixture
{
  public:
    SgxTestFixture() {}

    ~SgxTestFixture() {}

  protected:
    std::shared_ptr<sgx::WAMREnclave> wamrEnclave = nullptr;
};

TEST_CASE("Test enclave set up and tear down", "[sgx]")
{
    std::shared_ptr<sgx::WAMREnclave> wamrEnclave =
      sgx::acquireGlobalWAMREnclaveLock();

    wamrEnclave->tearDownSgxEnclave();

    REQUIRE(!wamrEnclave->isSgxEnclaveSetUp());

    // Initialise the global enclave
    wamrEnclave->initSgxEnclave();

    REQUIRE(wamrEnclave->isSgxEnclaveSetUp());

    wamrEnclave->tearDownSgxEnclave();

    REQUIRE(wamrEnclave->getId() == 0);

    wamrEnclave = nullptr;
    sgx::releaseGlobalWAMREnclaveLock();
}

TEST_CASE("Test SGX crypto checks", "[sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto());
}

TEST_CASE("Test SGX module store", "[sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxModuleStore());
}
}
