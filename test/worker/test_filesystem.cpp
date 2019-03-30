#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>


namespace tests {
    TEST_CASE("Test gedents", "[worker]") {
        cleanSystem();

        std::string dirName;
        std::string expected;
        SECTION("Valid directory") {
            dirName = "lib/python3.7";
            expected = "include,lib,..,.,";
        }

        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("getdents");
        msg.set_resultkey("getdents_test");

        const std::string result = execFunctionWithStringResult(msg);
        REQUIRE(result == expected);

        conf.reset();
    }

    TEST_CASE("Test errno with stat64", "[worker]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("errno");
        msg.set_resultkey("errno_test");

        // Will fail if invalid
        execFunction(msg);

        conf.reset();
    }
}