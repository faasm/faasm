#include <catch/catch.hpp>

#include <util/config.h>
#include <util/func.h>
#include <utils.h>

namespace tests {
    TEST_CASE("Test rust function works", "[worker]") {
        cleanSystem();

        // TODO - WASI - get Rust working again
        // message::Message call = util::messageFactory("rust", "hello");
        // execFunction(call);
    }
}