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

    TEST_CASE("Test abort", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "abort");
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
}