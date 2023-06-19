#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {

class ErrorCheckFixture : public MultiRuntimeFunctionExecTestFixture
{
  public:
    void checkError(const std::string& funcName, const std::string& expectedMsg)
    {
        auto req = setUpContext("errors", funcName);
        // Let the executor know its fine if the return value is non-zero
        bool requireSuccess = false;
        faabric::Message result =
          executeWithPool(req, EXECUTE_POOL_TIMEOUT_MS, requireSuccess).at(0);

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
                 "[faaslet]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    // 21/02/2023 - See bytecodealliance/wasm-micro-runtime#1979
    // SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkError("ret_one", "Call failed (return value=1)");
}
}
