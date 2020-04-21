#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test zygote function works", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "zygote_check");
        execFunction(msg);
    }

    TEST_CASE("Test repeat execution of zygote function", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "zygote_check");
        checkMultipleExecutions(msg, 4);
    }
}