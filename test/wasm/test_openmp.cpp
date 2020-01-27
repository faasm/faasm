#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test running OpenMP checks", "[wasm]") {
        cleanSystem();
        message::Message msg = util::messageFactory("omp", "checks");
        execFunction(msg);
    }
}