#include <catch2/catch.hpp>

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

TEST_CASE("Test emulation", "[emulator]")
{
    cleanSystem();

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
}
}
