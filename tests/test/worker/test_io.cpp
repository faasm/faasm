#include <catch/catch.hpp>
#include "utils.h"

extern "C" {
#include <emulator/emulator_api.h>
}

#include <util/config.h>
#include <worker/WorkerThreadPool.h>
#include <emulator/emulator.h>

using namespace worker;

namespace tests {
    TEST_CASE("Test fixed input with colon", "[worker]") {
        cleanSystem();
        message::Message call = util::messageFactory("demo", "check_input");
        call.set_inputdata("http://www.foobar.com");

        execFunction(call);
    }

    TEST_CASE("Test file operations", "[worker]") {
        cleanSystem();
        message::Message call = util::messageFactory("demo", "file");
        execFunction(call);
    }

    TEST_CASE("Test execution of echo function", "[worker]") {
        cleanSystem();
        message::Message call = util::messageFactory("demo", "echo");
        std::string inputData = "http://www.testinput/foo.com";
        call.set_inputdata(inputData.c_str());

        const std::string actual = execFunctionWithStringResult(call);
        REQUIRE(actual == inputData);
    }

    TEST_CASE("Test capturing stdout", "[worker]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        message::Message call = util::messageFactory("demo", "stdout");

        std::string expected;

        SECTION("Capture off") {
            conf.captureStdout = "off";
            expected = "Normal Faasm output";
        }

        SECTION("Capture on") {
            conf.captureStdout = "on";
            expected = "i=7 s=8 f=7.89\n"
                       "FloatA=1.234 FloatB=7.891\n"
                       "Out: I am output\n"
                       "Unformatted output\n\n"
                       "Normal Faasm output";
        }

        const std::string actual = execFunctionWithStringResult(call);
        REQUIRE(actual == expected);
    }
}