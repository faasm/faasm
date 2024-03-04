#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getenv",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "getenv");

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
                 "Test conf flags",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "conf_flags");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test exit", "[faaslet]")
{
    auto req = setUpContext("demo", "exit");

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
                 "Test optarg",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "optarg");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test sysconf",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "sysconf");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test uname", "[faaslet]")
{
    auto req = setUpContext("demo", "uname");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getpwuid",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "getpwuid");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getcwd",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "getcwd");

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test argc/argv",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "argc_argv_test");
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline("alpha B_eta G$mma d3-lt4");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    /* 04/03/2023 - This test is failing in hardware mode
    #ifndef FAASM_SGX_DISABLED_MODE
        SECTION("SGX") { execSgxFunction(msg); }
    #endif
    */

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test waitpid",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "waitpid");

    executeWithPool(req);
}
}
