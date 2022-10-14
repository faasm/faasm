#include <catch2/catch.hpp>

#include <conf/FaasmConfig.h>
#ifndef FAASM_SGX_DISABLED_MODE
#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#endif

#include "faasm_fixtures.h"

namespace tests {
class SgxTestFixture
{
  public:
    SgxTestFixture() { sgx::checkSgxSetup(); }

    ~SgxTestFixture() { sgx::tearDownEnclave(); }
};

class SgxInternalTestFixture
  : public SgxTestFixture
  , public ExecutorContextTestFixture
{
  public:
    SgxInternalTestFixture() {}

    ~SgxInternalTestFixture() {}
};

TEST_CASE("Test enclave set up and tear down", "[.][sgx]")
{
    sgx::tearDownEnclave();

    REQUIRE(sgx::getGlobalEnclaveId() == 0);

    // Initialise the global enclave
    sgx::checkSgxSetup();

    REQUIRE(sgx::getGlobalEnclaveId() != 0);

    sgx::tearDownEnclave();

    REQUIRE(sgx::getGlobalEnclaveId() == 0);
}

TEST_CASE_METHOD(SgxTestFixture, "Test SGX crypto checks", "[.][sgx]")
{
    REQUIRE_NOTHROW(sgx::checkSgxCrypto());
}

TEST_CASE_METHOD(SgxInternalTestFixture, "Test SGX-WAMR sbrk", "[sgx]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "sgx";

    auto req = setUpContext("demo", "hello");
    faabric::Message& call = req->mutable_messages()->at(0);
    wasm::EnclaveInterface enclaveInterface;
    enclaveInterface.bindToFunction(call);

    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallRunInternalTest(sgx::getGlobalEnclaveId(),
                           &returnValue,
                           enclaveInterface.interfaceId,
                           "sbrk");
    sgx::processECallErrors(
      "Error running internal test: sbrk", sgxReturnValue, returnValue);
    REQUIRE(returnValue == FAASM_SGX_SUCCESS);
}
}
