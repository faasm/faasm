#include <catch/catch.hpp>

#include "utils.h"

#include <faabric/util/environment.h>

#include <faaslet/FaasletPool.h>
#include <emulator/emulator.h>

using namespace faaslet;

namespace tests {
    TEST_CASE("Test function chaining", "[faaslet]") {
        faabric::Message call = faabric::utilmessageFactory("demo", "chain");
        execFuncWithPool(call, false, 2);
    }

    TEST_CASE("Test named function chaining", "[faaslet]") {
        faabric::Message call = faabric::utilmessageFactory("demo", "chain_named_a");
        execFuncWithPool(call, false, 2);
    }

    TEST_CASE("Test appended state", "[faaslet]") {
        faabric::Message call = faabric::utilmessageFactory("demo", "state_append");
        execFuncWithPool(call, false, 1);
    }
}