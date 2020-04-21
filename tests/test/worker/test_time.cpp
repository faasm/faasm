#include <catch/catch.hpp>

#include "utils.h"

namespace tests {
    TEST_CASE("Test time progresses", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "gettime");
        execFunction(msg);
    }
}
