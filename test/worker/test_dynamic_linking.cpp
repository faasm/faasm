#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    // NOTE: if this is failing make sure you've set up the runtime root
    // this is necessary for the function to dynamically load the modules
    TEST_CASE("Test e2e execution of dynamic linking modules", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "dynlink");
        execFunction(msg);
    }

    TEST_CASE("Test repeat execution of dynamically linked modules", "[worker]") {
        message::Message msg = util::messageFactory("demo", "dynlink");
        checkMultipleExecutions(msg, 3);
    }
}