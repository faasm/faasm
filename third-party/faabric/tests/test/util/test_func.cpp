#include <catch/catch.hpp>

#include <util/func.h>
#include <util/config.h>
#include <boost/filesystem.hpp>
#include <util/clock.h>

using namespace boost::filesystem;

namespace tests {
    TEST_CASE("Test message factory", "[util]") {
        const faabric::Message msg = util::messageFactory("demo", "echo");
        REQUIRE(msg.user() == "demo");
        REQUIRE(msg.function() == "echo");
        REQUIRE(msg.id() > 0);
        REQUIRE(!msg.statuskey().empty());
        REQUIRE(!msg.resultkey().empty());
    }

    TEST_CASE("Test retrieving function paths", "[util]") {
        util::SystemConfig &conf = util::getSystemConfig();

        std::string funcName = "beta";

        faabric::Message call;
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
        faabric::Message invalidCall;
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
        faabric::Message validCall;
        validCall.set_user("demo");
        validCall.set_function("echo");

        REQUIRE(util::isValidFunction(validCall));
    }

    TEST_CASE("Test adding id to message", "[util]") {
        faabric::Message msgA;
        faabric::Message msgB;

        REQUIRE(msgA.id() == 0);
        REQUIRE(msgA.resultkey().empty());
        REQUIRE(msgA.statuskey().empty());

        REQUIRE(msgB.id() == 0);
        REQUIRE(msgB.resultkey().empty());
        REQUIRE(msgB.statuskey().empty());

        util::setMessageId(msgA);
        util::setMessageId(msgB);

        REQUIRE(msgA.id() > 0);
        REQUIRE(msgB.id() > 0);
        REQUIRE(msgB.id() > msgA.id());

        std::string expectedResultKeyA = std::string("result_" + std::to_string(msgA.id()));
        std::string expectedStatusKeyA = std::string("status_" + std::to_string(msgA.id()));
        REQUIRE(msgA.resultkey() == expectedResultKeyA);
        REQUIRE(msgA.statuskey() == expectedStatusKeyA);

        std::string expectedResultKeyB = std::string("result_" + std::to_string(msgB.id()));
        std::string expectedStatusKeyB = std::string("status_" + std::to_string(msgB.id()));
        REQUIRE(msgB.resultkey() == expectedResultKeyB);
        REQUIRE(msgB.statuskey() == expectedStatusKeyB);
    }

    TEST_CASE("Test adding ID to message with an existing ID") {
        faabric::Message msg;
        util::setMessageId(msg);

        int originalId = msg.id();
        std::string originalStatusKey = msg.statuskey();
        std::string originalResultKey = msg.resultkey();

        util::setMessageId(msg);
        int afterId = msg.id();
        std::string afterStatusKey = msg.statuskey();
        std::string afterResultKey = msg.resultkey();

        REQUIRE(afterId == originalId);
        REQUIRE(afterStatusKey == originalStatusKey);
        REQUIRE(afterResultKey == originalResultKey);
    }


    TEST_CASE("Test timestamp added to message") {
        faabric::Message msg;
        unsigned int msgId = 1234;

        // Epoch millis on 27/07/2020
        long baselineTimestamp = 1595862090240;
        msg.set_id(msgId);

        SECTION("Existing timestamp") {
            long expectedTimestamp = 999888;
            msg.set_timestamp(expectedTimestamp);

            util::setMessageId(msg);
            REQUIRE(msg.timestamp() == expectedTimestamp);
        }

        SECTION("Zero existing timestamp") {
            msg.set_timestamp(0);
            util::setMessageId(msg);
            REQUIRE(msg.timestamp() > baselineTimestamp);
        }

        SECTION("No existing timestamp") {
            util::setMessageId(msg);
            REQUIRE(msg.timestamp() > baselineTimestamp);
        }
    }

    TEST_CASE("Check message with ID already set still gets result key and status key", "[util]") {
        faabric::Message msg;
        int msgId = 1234;
        msg.set_id(msgId);
        msg.set_statuskey("");
        msg.set_resultkey("");

        util::setMessageId(msg);
        REQUIRE(msg.statuskey() == util::statusKeyFromMessageId(msgId));
        REQUIRE(msg.resultkey() == util::resultKeyFromMessageId(msgId));
    }

    TEST_CASE("Test creating async response") {
        faabric::Message msg = util::messageFactory("foo", "bar");

        const std::string expected = std::to_string(msg.id());
        const std::string actual = util::buildAsyncResponse(msg);

        REQUIRE(expected == actual);
    }
}