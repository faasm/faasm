#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/executor/FaabricPool.h>
#include <faabric/util/config.h>

using namespace faaslet;

namespace tests {
TEST_CASE("Test fixed input with colon", "[faaslet]")
{
    cleanSystem();
    faabric::Message call =
      faabric::util::messageFactory("demo", "check_input");
    call.set_inputdata("http://www.foobar.com");

    execFunction(call);
}

TEST_CASE("Test execution of echo function", "[faaslet]")
{
    cleanSystem();
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    std::string inputData = "http://www.testinput/foo.com";
    call.set_inputdata(inputData.c_str());

    const std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == inputData);
}

TEST_CASE("Test capturing stdout", "[faaslet]")
{
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    faabric::Message call = faabric::util::messageFactory("demo", "stdout");

    std::string expected;

    SECTION("Capture off")
    {
        call.set_inputdata("21");
        conf.captureStdout = "off";
        expected = "Normal Faasm output";
    }

    SECTION("Capture on")
    {
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
}
