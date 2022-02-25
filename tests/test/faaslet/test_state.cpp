#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faaslet/Faaslet.h>

using namespace faaslet;

namespace tests {

class StateFuncTestFixture : public FunctionExecTestFixture
{
  public:
    void checkStateExample(const std::string& funcName,
                           const std::string& keyName,
                           const std::vector<uint8_t>& expectedOutput,
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
        faabric::Message result = sch.getFunctionResult(call.id(), 1);
        REQUIRE(result.returnvalue() == 0);
        std::vector<uint8_t> outputBytes =
          faabric::util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);

        const std::shared_ptr<faabric::state::StateKeyValue>& kv =
          faabric::state::getGlobalState().getKV("demo", keyName, 0);
        REQUIRE(kv->size() == expectedState.size());
        std::vector<uint8_t> actual(kv->size(), 0);
        kv->get(actual.data());
        REQUIRE(actual == expectedState);

        m.shutdown();
    }
};

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test repeat invocation with state",
                 "[state]")
{
    // Set up the function call
    faabric::Message call = faabric::util::messageFactory("demo", "increment");

    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);

    // Check result
    faabric::Message resultA = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultA.returnvalue() == 0);
    REQUIRE(resultA.outputdata() == "Counter: 001");

    // Call the function a second time, the state should have been incremented
    call.set_id(0);
    faabric::util::setMessageId(call);

    sch.callFunction(call);

    faabric::Message resultB = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultB.returnvalue() == 0);
    REQUIRE(resultB.outputdata() == "Counter: 002");

    m.shutdown();
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test asynchronous state", "[state]")
{
    checkStateExample(
      "state_async", "state_async_example", { 1, 1, 1, 1 }, { 3, 2, 1, 0 });
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test offset state", "[state]")
{
    checkStateExample("state_offset",
                      "state_offset_example",
                      { 5, 5, 6, 6, 4 },
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
    conf::getFaasmConfig().reset();
    auto req = setUpContext("demo", "state_file");
    execFunction(req);
}

TEST_CASE_METHOD(StateFuncTestFixture, "Test appended state", "[state]")
{
    auto req = setUpContext("demo", "state_append");
    faabric::Message& call = req->mutable_messages()->at(0);
    execFuncWithPool(call);
}
}
