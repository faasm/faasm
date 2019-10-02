#include <catch/catch.hpp>

#include "utils.h"
#include <ibm/IBMEndpoint.h>

#include <scheduler/GlobalMessageBus.h>
#include <util/json.h>

using namespace Pistache;

namespace tests {
    TEST_CASE("Test valid ibm invocation", "[ibm]") {
        cleanSystem();

        ibm::IBMEndpoint endpoint;
    }
}