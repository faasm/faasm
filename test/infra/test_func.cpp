#include <catch/catch.hpp>
#include <infra/infra.h>
#include <proto/faasm.pb.h>


namespace tests {

    TEST_CASE("Test retrieving function path", "[infra]") {
        message::FunctionCall call;

        call.set_user("jimmy");
        call.set_function("myfun");

        std::string expected = "/usr/local/code/faasm/wasm/jimmy/myfun/function.wasm";
        REQUIRE(expected == infra::getFunctionFile(call));
    }
}