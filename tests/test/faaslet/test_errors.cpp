#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/InMemoryMessageQueue.h>

using namespace faaslet;

namespace tests {
void checkError(const std::string& funcName, const std::string& expectedMsg)
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("errors", funcName);

    faabric::Message result = execErrorFunction(call);

    // Get result
    REQUIRE(result.returnvalue() > 0);

    if (expectedMsg.empty()) {
        return;
    }

    const std::string actualOutput = result.outputdata();
    bool messageIsFound = false;
    if (actualOutput.find(expectedMsg) != std::string::npos) {
        messageIsFound = true;
    }

    if (!messageIsFound) {
        printf(
          "%s not found in %s\n", expectedMsg.c_str(), actualOutput.c_str());
    }

    REQUIRE(messageIsFound);
}

TEST_CASE("Test non-zero return code is error", "[wasm]")
{
    checkError("ret_one", "Call failed (return value=1)");
}
}
