#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test pthreads", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "threads");
        execFunction(msg);
    }
}