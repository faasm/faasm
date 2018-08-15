#include <catch/catch.hpp>

#include <infra/infra.h>
#include <boost/filesystem.hpp>

namespace tests {

    TEST_CASE("Test retrieving function path", "[infra]") {
        message::FunctionCall call;

        call.set_user("jimmy");
        call.set_function("myfun");

        std::string dummyRoot = "/tmp/foo/bar";
        boost::filesystem::create_directories(dummyRoot + "/wasm");

        char envVarBuf[30];
        sprintf(envVarBuf, "FUNC_ROOT=%s", dummyRoot.c_str());
        putenv(envVarBuf);

        std::string expected = "/tmp/foo/bar/wasm/jimmy/myfun/function.wasm";
        REQUIRE(expected == infra::getFunctionFile(call));

        // Clear up afterwards
        char cleanRoot[] = "FUNC_ROOT";
        unsetenv(cleanRoot);
    }
}