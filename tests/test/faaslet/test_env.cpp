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
    faabric::Message msg = faabric::util::messageFactory("demo", "getenv");

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test conf flags",
                 "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "conf_flags");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test exit",
                 "[faaslet][wamr]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "exit");

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test optarg",
                 "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "optarg");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test sysconf",
                 "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "sysconf");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test uname", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "uname");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getpwuid",
                 "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "getpwuid");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test getcwd",
                 "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "getcwd");
    execFunction(msg);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test argc/argv",
                 "[faaslet][wamr]")
{
    faabric::Message msg =
      faabric::util::messageFactory("demo", "argc_argv_test");
    msg.set_cmdline("alpha B_eta G$mma d3-lt4");

    SECTION("WAVM") { execFunction(msg); }

    SECTION("WAMR") { execWamrFunction(msg); }
}
}
