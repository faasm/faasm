#include <catch/catch.hpp>

#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/utils.h>

namespace tests {
    TEST_CASE("Test rust function works", "[faaslet]") {
        cleanSystem();

        // TODO - WASI - get Rust working again
        // faabric::Message call = faabric::utilmessageFactory("rust", "hello");
        // execFunction(call);
    }
}