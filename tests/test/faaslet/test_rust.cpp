#include <catch/catch.hpp>

#include <util/config.h>
#include <util/func.h>
#include <utils.h>

namespace tests {
    TEST_CASE("Test rust function works", "[faaslet]") {
        cleanSystem();

        // TODO - WASI - get Rust working again
        // faabric::Message call = util::messageFactory("rust", "hello");
        // execFunction(call);
    }
}