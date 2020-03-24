#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test memcpy", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }

    TEST_CASE("Test memmove", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }

    TEST_CASE("Test calloc", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }
}