#include <catch/catch.hpp>

#include <worker/worker.h>


namespace tests {
    static infra::Redis redis;

    void checkError(const std::string &funcName, const std::string &expectedMsg) {
        redis.flushAll();

        message::FunctionCall call;
        call.set_user("errors");
        call.set_function(funcName);
        call.set_resultkey("error_test");

        worker::execFunction(1, call);

        // Get result
        message::FunctionCall result = redis.getFunctionResult(call);
        REQUIRE(!result.success());

        const std::string &actualOutput = result.outputdata();
        bool isFound = false;
        if (actualOutput.find(expectedMsg) != std::string::npos) {
            isFound = true;
        }

        REQUIRE(isFound);
    }

    TEST_CASE("Test opening path in write mode", "[wasm]") {
        checkError("open_write", "open fd in non read-only");
    }

    TEST_CASE("Test connecting to socket not owned by function", "[wasm]") {
        checkError("socket", "fd not owned by this function");
    }
}