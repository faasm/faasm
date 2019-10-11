#include <catch/catch.hpp>

#include <util/func.h>
#include <util/config.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace tests {
    TEST_CASE("Test message factory", "[util]") {
        const message::Message msg = util::messageFactory("demo", "echo");
        REQUIRE(msg.user() == "demo");
        REQUIRE(msg.function() == "echo");
        REQUIRE(msg.id() > 0);
        REQUIRE(!msg.statuskey().empty());
        REQUIRE(!msg.resultkey().empty());
    }

    TEST_CASE("Test retrieving function paths", "[util]") {
        util::SystemConfig &conf = util::getSystemConfig();

        std::string funcName = "beta";

        message::Message call;
        call.set_user("alpha");
        call.set_function(funcName);

        // Set up dummy environment
        std::string dummyRoot = "/tmp/foo/bar";
        std::string orig = conf.functionDir;
        conf.functionDir = dummyRoot;

        // Create root path
        path funcDir(dummyRoot);
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
        conf.functionDir = orig;
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

    TEST_CASE("Test adding id to message", "[util]") {
        message::Message msg;

        REQUIRE(msg.id() == 0);
        REQUIRE(msg.resultkey().empty());
        REQUIRE(msg.statuskey().empty());

        util::setMessageId(msg);

        REQUIRE(msg.id() > 0);

        std::string expectedResultKey = std::string("result_" + std::to_string(msg.id()));
        std::string expectedStatusKey = std::string("status_" + std::to_string(msg.id()));
        REQUIRE(msg.resultkey() == expectedResultKey);
        REQUIRE(msg.statuskey() == expectedStatusKey);
    }

    TEST_CASE("Test converting message to python", "[util]") {
        message::Message msg = util::messageFactory("foo", "bar");

        util::convertMessageToPython(msg);

        REQUIRE(msg.ispython());
        REQUIRE(msg.inputdata() == "foo/bar/function.py");
        REQUIRE(msg.user() == PYTHON_USER);
        REQUIRE(msg.function() == PYTHON_FUNC);
    }
}