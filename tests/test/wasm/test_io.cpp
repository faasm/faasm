#include "utils.h"
#include <catch2/catch.hpp>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {

TEST_CASE("Test printf", "[wasm]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "print");
    execFunction(call);
}

TEST_CASE("Test emscripten", "[wasm]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "emscripten_check");
    execFunction(call);
}
}
