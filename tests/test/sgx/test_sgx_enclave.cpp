#include <catch2/catch.hpp>

#include <sgx/WamrEnclave.h>
#include <sgx/system.h>

namespace tests {
class SgxTestFixture
{
  public:
    SgxTestFixture()
      : wamrEnclave(sgx::getWamrEnclave())
    {}

    ~SgxTestFixture() { wamrEnclave.tearDown(); }

  protected:
    sgx::WamrEnclave& wamrEnclave;
};

TEST_CASE("Test enclave set up and tear down", "[sgx]")
{
    sgx::WamrEnclave& wamrEnclave = sgx::getWamrEnclave();

    wamrEnclave.tearDown();

    REQUIRE(!wamrEnclave.isSetUp());

    // Initialise the global enclave
    wamrEnclave.init();

    REQUIRE(wamrEnclave.isSetUp());

    wamrEnclave.tearDown();

    REQUIRE(wamrEnclave.getId() == 0);
}

TEST_CASE_METHOD(SgxTestFixture, "Test enclave is not re-created", "[sgx]")
{
    REQUIRE(wamrEnclave.getId() == sgx::getWamrEnclave().getId());
}

TEST_CASE_METHOD(SgxTestFixture, "Test SGX crypto checks", "[sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto());
}
}
