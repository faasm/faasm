#include <catch/catch.hpp>

extern "C"
{
#include <emulator/emulator_api.h>
#include <faasm/host_interface.h>
}

#include "utils.h"

#include <emulator/emulator.h>
#include <faabric/util/json.h>
#include <faabric/util/state.h>
#include <faasm/core.h>

namespace tests {
void _doEmulationTest(const std::string& hostType)
{
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string oldHostType = conf.hostType;
    conf.hostType = hostType;

    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
    long dummyLen = dummyBytes.size();

    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    SECTION("Output data")
    {
        setEmulatedMessage(call);
        faasmSetOutput(dummyBytes.data(), dummyLen);
        const std::vector<uint8_t> actual = getEmulatorOutputData();
        REQUIRE(actual == dummyBytes);
    }

    SECTION("Input data")
    {
        call.set_inputdata(dummyBytes.data(), dummyBytes.size());
        setEmulatedMessage(call);

        long actual = faasmGetInputSize();
        REQUIRE(actual == dummyLen);

        std::vector<uint8_t> actualBytes(dummyLen);
        faasmGetInput(actualBytes.data(), dummyLen);

        REQUIRE(actualBytes == dummyBytes);
    }

    SECTION("Read/ write state")
    {
        setEmulatedMessage(call);

        std::string key = "foobar";
        faasmWriteState(key.c_str(), dummyBytes.data(), dummyLen);
        faasmPushState(key.c_str());

        // Check reading from state
        std::vector<uint8_t> actual(dummyLen);
        faasmReadState(key.c_str(), actual.data(), dummyLen);
        REQUIRE(actual == dummyBytes);
    }

    SECTION("Read/ write state offset")
    {
        setEmulatedMessage(call);

        std::string key = "foobar_off";
        std::vector<uint8_t> offsetData = { 7, 6, 5 };
        long offset = 2;
        long dataLen = 3;
        faasmWriteStateOffset(
          key.c_str(), dummyLen, offset, offsetData.data(), dataLen);
        faasmPushState(key.c_str());

        // Check reading from state
        std::vector<uint8_t> actual(dataLen);
        faasmPullState(key.c_str(), dummyLen);
        faasmReadStateOffset(
          key.c_str(), dummyLen, offset, actual.data(), dataLen);
        REQUIRE(actual == offsetData);
    }

    conf.hostType = oldHostType;
}

TEST_CASE("Test state emulation simple", "[emulator]")
{
    _doEmulationTest("knative");
}

TEST_CASE("Test state emulation complex", "[emulator]")
{
    _doEmulationTest("default");
}

TEST_CASE("Test emulator setting function result", "[emulator]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    faabric::util::setMessageId(call);

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalHostType = conf.hostType;
    conf.hostType = "knative";

    bool success = 0;
    bool useJson = false;
    SECTION("Failure")
    {
        success = false;

        SECTION("Failure and JSON") { useJson = true; }
        SECTION("Failure and not JSON") { useJson = false; }
    }

    SECTION("Success")
    {
        success = true;

        SECTION("Success and JSON") { useJson = true; }
        SECTION("Success and not JSON") { useJson = false; }
    }

    unsigned int messageId = 0;
    if (useJson) {
        const std::string jsonStr = faabric::util::messageToJson(call);
        messageId = setEmulatedMessageFromJson(jsonStr.c_str());
    } else {
        messageId = setEmulatedMessage(call);
    }

    REQUIRE(messageId > 0);
    emulatorSetCallStatus(success);

    // Call the await call function directly
    int resultCode = __faasm_await_call(call.id());

    if (success) {
        REQUIRE(resultCode == 0);
    } else {
        REQUIRE(resultCode == 1);
    }

    conf.hostType = originalHostType;
}
}