#include <catch/catch.hpp>

#include "utils.h"

#include <faabric/util/config.h>

namespace tests {
    TEST_CASE("Test rust function works", "[faaslet]") {
        cleanSystem();

        // TODO - WASI - get Rust working again
        // faabric::Message call = faabric::util::messageFactory("rust", "hello");
        // execFunction(call);
    }
}