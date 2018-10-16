#include <catch/catch.hpp>

#include <infra/infra.h>
#include <util/util.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace tests {

    TEST_CASE("Test retrieving function paths", "[infra]") {
        message::FunctionCall call;
        call.set_user("alpha");
        call.set_function("beta");

        // Set up dummy environment
        std::string dummyRoot = "/tmp/foo/bar";
        util::setEnvVar("FUNC_ROOT", dummyRoot);

        // Create root path
        path funcDir(dummyRoot);
        funcDir.append("wasm");
        create_directories(funcDir);

        // Make sure existing directories don't exist
        funcDir.append("alpha/beta");
        if (exists(funcDir)) {
            remove_all(funcDir);
        }

        path funcFile = funcDir;
        funcFile.append("function.wasm");

        const std::string actual = infra::getFunctionFile(call);

        // Check directory has been created and function file is as expected
        REQUIRE(exists(funcDir));
        REQUIRE(actual == funcFile.string());

        // Clear up afterwards
        util::unsetEnvVar("FUNC_ROOT");
    }

    TEST_CASE("Test valid function check returns false for invalid function", "[infra]") {
        // Check false for uninitialised call
        message::FunctionCall invalidCall;
        REQUIRE(!infra::isValidFunction(invalidCall));

        // Check false for call initialised with invalid values
        invalidCall.set_user("foo");
        invalidCall.set_function("bar");
        REQUIRE(!infra::isValidFunction(invalidCall));

        // Check false for call with valid user but invalid function
        invalidCall.set_user("demo");
        invalidCall.set_function("bar");
        REQUIRE(!infra::isValidFunction(invalidCall));
    }

    TEST_CASE("Test valid function check returns true for valid function", "[infra]") {
        // Check true for valid call
        message::FunctionCall validCall;
        validCall.set_user("demo");
        validCall.set_function("echo");

        REQUIRE(infra::isValidFunction(validCall));
    }
}