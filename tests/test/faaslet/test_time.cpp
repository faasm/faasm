#include <catch/catch.hpp>

#include "utils.h"

namespace tests {
    TEST_CASE("Test time progresses", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = faabric::utilmessageFactory("demo", "gettime");
        execFunction(msg);
    }
}
