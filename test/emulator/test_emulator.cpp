#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <emulator/emulator.h>
#include <faasm/core.h>
#include <util/state.h>


namespace tests {
    void _doEmulationTest(const std::string &hostType) {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        std::string oldHostType = conf.hostType;
        conf.hostType = hostType;

        redis::Redis &redisState = redis::Redis::getState();

        std::vector<uint8_t> dummyBytes = {0, 1, 2, 3, 4, 5, 6, 7, 8};
        long dummyLen = dummyBytes.size();

        SECTION("Output data") {
            faasmSetOutput(dummyBytes.data(), dummyLen);
            const std::vector<uint8_t> actual = getEmulatorOutputData();
            REQUIRE(actual == dummyBytes);
        }

        SECTION("Input data") {
            setEmulatorInputData(dummyBytes);

            long actual = faasmGetInputSize();
            REQUIRE(actual == dummyLen);

            std::vector<uint8_t> actualBytes(dummyLen);
            faasmGetInput(actualBytes.data(), dummyLen);

            REQUIRE(actualBytes == dummyBytes);
        }

        SECTION("Read/ write state") {
            std::string key = "foobar";
            faasmWriteState(key.c_str(), dummyBytes.data(), dummyLen);
            faasmPushState(key.c_str());

            // Check written to Redis
            const std::string actualKey = util::keyForUser(getEmulatorUser(), key);
            const std::vector<uint8_t> actualRedis = redisState.get(actualKey);
            REQUIRE(actualRedis == dummyBytes);

            // Check reading from state
            std::vector<uint8_t> actual(dummyLen);
            faasmReadState(key.c_str(), actual.data(), dummyLen);
            REQUIRE(actual == dummyBytes);
        }

        SECTION("Read/ write state offset") {
            std::string key = "foobar_off";
            std::vector<uint8_t> offsetData = {7, 6, 5};
            long offset = 2;
            long dataLen = 3;
            faasmWriteStateOffset(key.c_str(), dummyLen, offset, offsetData.data(), dataLen);
            faasmPushState(key.c_str());

            // Check written to Redis
            const std::string actualKey = util::keyForUser(getEmulatorUser(), key);
            std::vector<uint8_t> actualRedis(dataLen);
            redisState.getRange(actualKey, actualRedis.data(), dataLen, offset, offset + dataLen - 1);
            REQUIRE(actualRedis == offsetData);

            // Check reading from state
            std::vector<uint8_t> actual(dataLen);
            faasmPullState(key.c_str(), dummyLen);
            faasmReadStateOffset(key.c_str(), dummyLen, offset, actual.data(), dataLen);
            REQUIRE(actual == offsetData);
        }

        conf.hostType = oldHostType;
    }

    TEST_CASE("Test state emulation simple", "[emulator]") {
        _doEmulationTest("knative");
    }

    TEST_CASE("Test state emulation complex", "[emulator]") {
        _doEmulationTest("default");
    }
}