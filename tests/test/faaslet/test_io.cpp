#include "faasm_fixtures.h"
#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/util/config.h>

#include <conf/FaasmConfig.h>

using namespace faaslet;

namespace tests {
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test fixed input with colon",
                 "[faaslet][wamr]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "check_input");
    call.set_inputdata("http://www.foobar.com");

    SECTION("WAVM") { execFunction(call); }

    SECTION("WAMR") { execWamrFunction(call); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test execution of echo function",
                 "[faaslet][wamr]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    std::string inputData = "http://www.testinput/foo.com";
    call.set_inputdata(inputData.c_str());

    conf::FaasmConfig& conf = conf::getFaasmConfig();
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    const std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == inputData);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test capturing stdout",
                 "[faaslet][wamr]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "stdout");
    std::string expected;

    SECTION("Capture off")
    {
        SECTION("WAVM") { conf.wasmVm = "wavm"; }

        SECTION("WAMR") { conf.wasmVm = "wamr"; }

        call.set_inputdata("21");
        conf.captureStdout = "off";
        expected = "Normal Faasm output";
    }

    SECTION("Capture on")
    {
        SECTION("WAVM") { conf.wasmVm = "wavm"; }

        SECTION("WAMR") { conf.wasmVm = "wamr"; }

        call.set_inputdata("23");
        conf.captureStdout = "on";
        expected = "Input value = 23\n"
                   "i=7 s=8 f=7.89\n"
                   "FloatA=28.393 FloatB=181.493\n"
                   "Out: I am output\n"
                   "Unformatted output\n\n"
                   "Normal Faasm output";
    }

    const std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == expected);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test capturing stderr",
                 "[faaslet]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "stderr");

    std::string expected;

    SECTION("Capture off")
    {
        SECTION("WAVM") { conf.wasmVm = "wavm"; }

        SECTION("WAMR") { conf.wasmVm = "wamr"; }

        conf.captureStdout = "off";
    }

    SECTION("Capture on")
    {
        SECTION("WAVM") { conf.wasmVm = "wavm"; }

        SECTION("WAMR") { conf.wasmVm = "wamr"; }

        conf.captureStdout = "on";
        expected = "stdin=0  stdout=1  stderr=2\n"
                   "This is for stderr\n\n";
    }

    const std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == expected);
}
}
