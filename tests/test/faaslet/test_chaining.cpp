#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/environment.h>

using namespace faaslet;

namespace tests {
TEST_CASE("Test function chaining", "[faaslet]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "chain");
    execFuncWithPool(call);
}

TEST_CASE("Test named function chaining", "[faaslet]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "chain_named_a");
    execFuncWithPool(call);
}

}
