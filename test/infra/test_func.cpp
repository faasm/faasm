#include <catch/catch.hpp>

#include <infra/infra.h>
#include <util/util.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace tests {

    TEST_CASE("Test retrieving function paths", "[infra]") {
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

        const std::string actual = infra::getFunctionFile(call);

        // Check directory has been created and function file is as expected
        REQUIRE(exists(funcDir));
        REQUIRE(actual == funcFile.string());

        // Clear up afterwards
        util::unsetEnvVar("FUNC_ROOT");
    }

    TEST_CASE("Test valid function check returns false for invalid function", "[infra]") {
        // Check false for uninitialised call
        message::Message invalidCall;
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
        message::Message validCall;
        validCall.set_user("demo");
        validCall.set_function("echo");

        REQUIRE(infra::isValidFunction(validCall));
    }

    TEST_CASE("Test building bind message", "[infra]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("blah blah");
        call.set_resultkey("foobar");

        REQUIRE(call.type() == message::Message_MessageType_CALL);

        message::Message bindMessage = infra::buildBindMessage(call, 10);
        REQUIRE(bindMessage.user() == call.user());
        REQUIRE(bindMessage.function() == call.function());
        REQUIRE(bindMessage.type() == message::Message_MessageType_BIND);
        REQUIRE(bindMessage.inputdata().empty());
        REQUIRE(bindMessage.resultkey().empty());
    }

    TEST_CASE("Test building prewarm message", "[infra]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("blah blah");
        call.set_resultkey("foobar");

        REQUIRE(call.type() == message::Message_MessageType_CALL);

        message::Message prewarmMessage = infra::buildPrewarmMessage(call);
        REQUIRE(prewarmMessage.user().empty());
        REQUIRE(prewarmMessage.function().empty());
        REQUIRE(prewarmMessage.type() == message::Message_MessageType_PREWARM);
        REQUIRE(prewarmMessage.inputdata().empty());
        REQUIRE(prewarmMessage.resultkey().empty());
    }
}