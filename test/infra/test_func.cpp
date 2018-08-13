#include <catch/catch.hpp>

#include <infra/infra.h>

namespace tests {

    TEST_CASE("Test retrieving function path", "[infra]") {
        message::FunctionCall call;

        call.set_user("jimmy");
        call.set_function("myfun");

        char dummyRoot[] = "FUNC_ROOT=/foo/bar";
        putenv(dummyRoot);

        std::string expected = "/foo/bar/wasm/jimmy/myfun/function.wasm";
        REQUIRE(expected == infra::getFunctionFile(call));

        // Clear up afterwards
        char cleanRoot[] = "FUNC_ROOT";
        unsetenv(cleanRoot);
    }
}