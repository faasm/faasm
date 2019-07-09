#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    // NOTE: if this is failing make sure you've set up the runtime root
    // this is necessary for the function to dynamically load the modules
    TEST_CASE("Test dynamic linking modules", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "dynlink");
        execFunction(msg);
    }
}