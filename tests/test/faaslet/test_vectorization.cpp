#include <catch/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
    TEST_CASE("Test eigen vectorization", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = faabric::utilmessageFactory("demo", "eigen_vec");
        execFunction(msg);
    }
}