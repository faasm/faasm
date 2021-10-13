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
      sgx::acquireGlobalWAMREnclave();

    wamrEnclave->tearDown();

    REQUIRE(!wamrEnclave->isSetUp());

    // Initialise the global enclave
    wamrEnclave->init();

    REQUIRE(wamrEnclave->isSetUp());

    wamrEnclave->tearDown();

    REQUIRE(wamrEnclave->getId() == 0);

    wamrEnclave = nullptr;
    sgx::releaseGlobalWAMREnclave();
}

TEST_CASE("Test SGX crypto checks", "[sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto());
}
}
