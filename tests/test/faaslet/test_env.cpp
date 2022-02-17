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
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test conf flags",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "conf_flags");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test exit",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "exit");
    faabric::Message& msg = req->mutable_messages()->at(0);

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test optarg",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "optarg");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test sysconf",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "sysconf");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test uname", "[faaslet]")
{
    auto req = setUpContext("demo", "uname");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getpwuid",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "getpwuid");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getcwd",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "getcwd");
    execFunction(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test argc/argv",
                 "[faaslet][wamr]")
{
    auto req = setUpContext("demo", "argc_argv_test");
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline("alpha B_eta G$mma d3-lt4");

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test waitpid",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "waitpid");
    execFunction(req);
}
}
