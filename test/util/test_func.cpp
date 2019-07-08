#include <catch/catch.hpp>

#include <util/func.h>
#include <util/environment.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace tests {

    TEST_CASE("Test retrieving function paths", "[util]") {
        std::string funcName;
        SECTION("Normal func name") {
            funcName = "beta";
        }

        SECTION("Func name with idx") {
            funcName = "beta__001__";
        }

        message::Message call;
        call.set_user("alpha");
        call.set_function(funcName);

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

    TEST_CASE("Test getting default function config", "[util]") {
        message::Message validCall;
        validCall.set_user("demo");
        validCall.set_function("echo");

        const util::FunctionConfig &actual = util::getFunctionConfig(validCall);
        REQUIRE(actual.runtime == "wasm");
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

    TEST_CASE("Test adding idx to message", "[util]") {
        message::Message msg;
        msg.set_function("foobar");

        REQUIRE(msg.idx() == 0);
        REQUIRE(msg.function() == "foobar");

        util::setMessageIdx(msg, 22);

        REQUIRE(msg.idx() == 22);
        REQUIRE(msg.function() == "foobar__022__");
    }

    TEST_CASE("Test adding idx to function name", "[util]") {
        REQUIRE(util::addIdxToFunction("foobar", 0) == "foobar");
        REQUIRE(util::addIdxToFunction("foobar", 1) == "foobar__001__");
        REQUIRE(util::addIdxToFunction("foobar", 5) == "foobar__005__");
        REQUIRE(util::addIdxToFunction("foobar", 11) == "foobar__011__");
        REQUIRE(util::addIdxToFunction("foobar", 321) == "foobar__321__");
    }

    TEST_CASE("Test stripping idx from funciton name", "[util]") {
        REQUIRE(util::stripIdxFromFunction("foobar") == "foobar");
        REQUIRE(util::stripIdxFromFunction("foobar123") == "foobar123");
        REQUIRE(util::stripIdxFromFunction("foobar__001__") == "foobar");
        REQUIRE(util::stripIdxFromFunction("foobar__123__") == "foobar");
    }
}