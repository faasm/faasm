#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>


namespace tests {
    TEST_CASE("Test getenv", "[worker]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("getenv");
        msg.set_resultkey("getenv_test");

        // Will fail if invalid
        execFunction(msg);

        conf.reset();
    }

    TEST_CASE("Test abort", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("abort");
        msg.set_resultkey("abort_test");

        execFunction(msg);
    }

    TEST_CASE("Test exit", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("exit");
        msg.set_resultkey("exit_test");

        execFunction(msg);
    }
}