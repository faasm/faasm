#include <catch2/catch.hpp>

#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>

#include "faasm_fixtures.h"

namespace tests {
class SgxInternalTestFixture : public MultiRuntimeFunctionExecTestFixture
{
  public:
    SgxInternalTestFixture()
    {
        faasmConf.wasmVm = "sgx";

        auto req = setUpContext("demo", "hello");
        faabric::Message& call = req->mutable_messages()->at(0);

        enclaveInterface.bindToFunction(call);
    }

    ~SgxInternalTestFixture() {}

    void doSgxInternalTest(const std::string& testName)
    {
        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = ecallRunInternalTest(
          enclaveInterface.getEnclaveId(), &returnValue, testName.c_str());
        sgx::processECallErrors("Error running internal test: hello-world",
                                sgxReturnValue,
                                returnValue);
        REQUIRE(returnValue == FAASM_SGX_SUCCESS);
    }

  protected:
    wasm::EnclaveInterface enclaveInterface;
};

TEST_CASE_METHOD(SgxInternalTestFixture, "Test enclave internal test", "[sgx]")
{
    doSgxInternalTest("hello-world");
}
}
