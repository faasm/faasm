#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test pointer to pointer", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "ptr_ptr");
        execFunction(msg);
    }

    TEST_CASE("Test sizes", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "sizes");
        execFunction(msg);
    }

    TEST_CASE("Test stack/ heap", "[faaslet]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "stackheap");
        execFunction(msg);
    }
}