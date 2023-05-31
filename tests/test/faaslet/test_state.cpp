#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/string_tools.h>
#include <faaslet/Faaslet.h>

using namespace faaslet;

namespace tests {

class StateFuncTestFixture : public FunctionExecTestFixture
{
  public:
    void checkStateExample(const std::string& funcName,
                           const std::string& keyName,
                           const std::string& expectedOutput,
                           const std::vector<uint8_t>& expectedState)
    {
        // Set up the function call
        faabric::Message call = faabric::util::messageFactory("demo", funcName);

        auto fac = std::make_shared<faaslet::FaasletFactory>();
        faabric::runner::FaabricMain m(fac);
        m.startRunner();

        faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
        sch.callFunction(call);

        // Check result
        faabric::Message result = sch.getFunctionResult(call, 1);
        REQUIRE(result.returnvalue() == 0);

        REQUIRE(result.outputdata() == expectedOutput);

        const std::shared_ptr<faabric::state::StateKeyValue>& kv =
          faabric::state::getGlobalState().getKV("demo", keyName, 0);
        REQUIRE(kv->size() == expectedState.size());
        std::vector<uint8_t> actual(kv->size(), 0);
        kv->get(actual.data());
        REQUIRE(actual == expectedState);

        m.shutdown();
    }
};

TEST_CASE_METHOD(StateFuncTestFixture, "Test asynchronous state", "[state]")
{
    checkStateExample(
      "state_async", "state_async_example", "equal", { 3, 2, 1, 0 });
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test offset state", "[state]")
{
    checkStateExample("state_offset",
                      "state_offset_example",
                      "success",
                      { 5, 5, 6, 6, 4, 5, 6 });
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test state size", "[state]")
{
    auto req = setUpContext("demo", "state_size");
    execFunction(req);
}

TEST_CASE_METHOD(StateFuncTestFixture,
                 "Test shared state write pointers",
                 "[state]")
{
    // Run the function to write
    checkCallingFunctionGivesBoolOutput("demo", "state_shared_write", true);

    // Run the function to read
    checkCallingFunctionGivesBoolOutput("demo", "state_shared_read", true);
}

TEST_CASE_METHOD(StateFuncTestFixture,
                 "Test shared state offset pointers",
                 "[state]")
{
    // Run the function to write
    checkCallingFunctionGivesBoolOutput(
      "demo", "state_shared_write_offset", true);

    // Run the function to read
    checkCallingFunctionGivesBoolOutput(
      "demo", "state_shared_read_offset", true);
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test writing file to state", "[state]")
{
    auto req = setUpContext("demo", "state_file");
    execFunction(req);
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test appended state", "[state]")
{
    auto req = setUpContext("demo", "state_append");
    faabric::Message& call = req->mutable_messages()->at(0);
    execFuncWithPool(call);
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test Pi estimate", "[state]")
{
    auto req = setUpContext("demo", "pi");
    faabric::Message& call = req->mutable_messages()->at(0);
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    std::string oldWasmVm = faasmConf.wasmVm;

    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    faabric::Message result = execFuncWithPool(call, true, 10000);
    std::string output = result.outputdata();
    REQUIRE(faabric::util::startsWith(output, "Pi estimate: 3.1"));

    faasmConf.wasmVm = oldWasmVm;
}
}
