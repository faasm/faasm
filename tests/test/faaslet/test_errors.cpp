#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/InMemoryMessageQueue.h>

using namespace faaslet;

namespace tests {

class ErrorCheckFixture : public MultiRuntimeFunctionExecTestFixture
{
  public:
    void checkError(const std::string& funcName, const std::string& expectedMsg)
    {
        auto req = setUpContext("errors", funcName);
        faabric::Message& call = req->mutable_messages()->at(0);
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
            printf("%s not found in %s\n",
                   expectedMsg.c_str(),
                   actualOutput.c_str());
        }

        REQUIRE(messageIsFound);
    }
};

TEST_CASE_METHOD(ErrorCheckFixture,
                 "Test non-zero return code is error",
                 "[wasm]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkError("ret_one", "Call failed (return value=1)");
}
}
