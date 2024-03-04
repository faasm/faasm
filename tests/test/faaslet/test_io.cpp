#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test fixed input with colon",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "check_input");
    auto& call = req->mutable_messages()->at(0);
    call.set_inputdata("http://www.foobar.com");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test execution of echo function",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "echo");
    faabric::Message& call = req->mutable_messages()->at(0);
    std::string inputData = "http://www.testinput/foo.com";
    call.set_inputdata(inputData.c_str());

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    const std::string actual = executeWithPool(req).at(0).outputdata();
    REQUIRE(actual == inputData);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test capturing stdout",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "stdout");
    faabric::Message& call = req->mutable_messages()->at(0);
    std::string expected;

    SECTION("Capture off")
    {
        SECTION("WAVM")
        {
            faasmConf.wasmVm = "wavm";
        }

        SECTION("WAMR")
        {
            faasmConf.wasmVm = "wamr";
        }

        call.set_inputdata("21");
        faasmConf.captureStdout = "off";
        expected = "Normal Faasm output";
    }

    SECTION("Capture on")
    {
        SECTION("WAVM")
        {
            faasmConf.wasmVm = "wavm";
        }

        SECTION("WAMR")
        {
            faasmConf.wasmVm = "wamr";
        }

        call.set_inputdata("23");
        faasmConf.captureStdout = "on";
        expected = "Input value = 23\n"
                   "i=7 s=8 f=7.89\n"
                   "FloatA=28.393 FloatB=181.493\n"
                   "Out: I am output\n"
                   "Unformatted output\n\n"
                   "Normal Faasm output";
    }

    const std::string actual = executeWithPool(req).at(0).outputdata();
    REQUIRE(actual == expected);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test capturing stderr",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "stderr");
    std::string expected;

    SECTION("Capture off")
    {
        SECTION("WAVM")
        {
            faasmConf.wasmVm = "wavm";
        }

        SECTION("WAMR")
        {
            faasmConf.wasmVm = "wamr";
        }

        faasmConf.captureStdout = "off";
    }

    SECTION("Capture on")
    {
        SECTION("WAVM")
        {
            faasmConf.wasmVm = "wavm";
        }

        SECTION("WAMR")
        {
            faasmConf.wasmVm = "wamr";
        }

        faasmConf.captureStdout = "on";
        expected = "stdin=0  stdout=1  stderr=2\n"
                   "This is for stderr\n\n";
    }

    const std::string actual = executeWithPool(req).at(0).outputdata();
    REQUIRE(actual == expected);
}
}
