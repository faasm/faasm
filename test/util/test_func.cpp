#include <catch/catch.hpp>

#include <util/func.h>
#include <util/environment.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace tests {

    TEST_CASE("Test retrieving function paths", "[util]") {
        message::Message call;
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

        const std::string actual = util::getFunctionFile(call);

        // Check directory has been created and function file is as expected
        REQUIRE(exists(funcDir));
        REQUIRE(actual == funcFile.string());

        // Clear up afterwards
        util::unsetEnvVar("FUNC_ROOT");
    }

    TEST_CASE("Test valid function check returns false for invalid function", "[util]") {
        // Check false for uninitialised call
        message::Message invalidCall;
        REQUIRE(!util::isValidFunction(invalidCall));

        // Check false for call initialised with invalid values
        invalidCall.set_user("foo");
        invalidCall.set_function("bar");
        REQUIRE(!util::isValidFunction(invalidCall));

        // Check false for call with valid user but invalid function
        invalidCall.set_user("demo");
        invalidCall.set_function("bar");
        REQUIRE(!util::isValidFunction(invalidCall));
    }

    TEST_CASE("Test valid function check returns true for valid function", "[util]") {
        // Check true for valid call
        message::Message validCall;
        validCall.set_user("demo");
        validCall.set_function("echo");

        REQUIRE(util::isValidFunction(validCall));
    }
}