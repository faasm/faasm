#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test memcpy", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }

    TEST_CASE("Test memmove", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }

    TEST_CASE("Test calloc", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "memcpy");
        execFunction(msg);
    }
}