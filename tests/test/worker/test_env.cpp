#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>


namespace tests {
    TEST_CASE("Test getenv", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "getenv");
        execFunction(msg);
    }

    TEST_CASE("Test conf flags", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "conf_flags");
        execFunction(msg);
    }

    TEST_CASE("Test exit", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "exit");
        execFunction(msg);
    }

    TEST_CASE("Test optarg", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "optarg");
        execFunction(msg);
    }

    TEST_CASE("Test sysconf", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "sysconf");
        execFunction(msg);
    }

    TEST_CASE("Test uname", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "uname");
        execFunction(msg);
    }

    TEST_CASE("Test getpwuid", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "getpwuid");
        execFunction(msg);
    }

    TEST_CASE("Test argc/argv", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "argc_argv_test");
        msg.set_cmdline("alpha B_eta G$mma d3-lt4");

        execFunction(msg);
    }
}