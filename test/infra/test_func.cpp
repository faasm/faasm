#include <catch/catch.hpp>

#include <infra/infra.h>
#include <util/util.h>
#include <boost/filesystem.hpp>

namespace tests {

    TEST_CASE("Test retrieving function path", "[infra]") {
        message::FunctionCall call;

        call.set_user("jimmy");
        call.set_function("myfun");

        std::string dummyRoot = "/tmp/foo/bar";
        boost::filesystem::create_directories(dummyRoot + "/wasm");

        util::setEnvVar("FUNC_ROOT", dummyRoot);

        std::string expected = "/tmp/foo/bar/wasm/jimmy/myfun/function.wasm";
        const std::string actual = infra::getFunctionFile(call);
        REQUIRE(actual == expected);

        // Clear up afterwards
        util::unsetEnvVar("FUNC_ROOT");
    }
}