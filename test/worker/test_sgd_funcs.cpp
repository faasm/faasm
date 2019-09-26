#include <catch/catch.hpp>

#include "utils.h"

#include <emulator/emulator.h>
#include <state/State.h>
#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>
#include <util/func.h>

#include <faasm/sgd.h>
#include <faasm/counter.h>


using namespace worker;

namespace tests {
    static void setUp(bool async) {
        cleanSystem();

        state::getGlobalState().forceClearAll();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.fullAsync = async;

        setEmulatorUser("sgd");
    }

    static void tearDown() {
        cleanSystem();
    }

    std::string execStrFunction(const std::string &funcName) {
        message::Message call = util::messageFactory("sgd", funcName);

        return execFunctionWithStringResult(call);
    }

    TEST_CASE("Test sgd losses", "[worker]") {
        bool async;
        SECTION("Synchronous") {
            async = false;
        }
        SECTION("Asynchronous") {
            async = true;
        }

        setUp(async);

        // Set up params
        SgdParams p;
        p.nEpochs = 5;
        p.fullAsync = async;
        faasm::writeParamsToState(PARAMS_KEY, p);

        // Set up memory
        std::vector<double> losses = {1000.5, 900.22, 20.1, 5.5, 99.999};
        std::vector<double> lossTimestamps = {10.0, 10.0 + 1.23, 10.0 + 12.34, 10.0 + 123.456, 10.0 + 1234.56};
        size_t nBytes = p.nEpochs * sizeof(double);
        faasmWriteState(LOSSES_KEY, reinterpret_cast<uint8_t *>(losses.data()), nBytes, async);
        faasmWriteState(LOSS_TIMESTAMPS_KEY, reinterpret_cast<uint8_t *>(lossTimestamps.data()), nBytes, async);

        // Call function
        std::string output = execStrFunction("sgd_loss");

        // Expected is relative timestamp - loss
        std::string expct = "0.0 - 1000.500, 1.230 - 900.219, 12.340 - 20.100, 123.456 - 5.500, 1234.560 - 99.999, ";
        REQUIRE(output == expct);

        tearDown();
    }

    TEST_CASE("Test sgd finished", "[worker]") {
        bool async;
        int epochCount = 5;
        int finishedCount;
        bool expectFinished;

        SECTION("Synchronous") {
            async = false;

            SECTION("Finished") {
                finishedCount = 5;
                expectFinished = true;
            }

            SECTION("Unfinished") {
                finishedCount = 3;
                expectFinished = false;
            }
        }
        SECTION("Asynchronous") {
            async = true;

            SECTION("Finished") {
                finishedCount = 5;
                expectFinished = true;
            }

            SECTION("Unfinished") {
                finishedCount = 3;
                expectFinished = false;
            }
        }

        setUp(async);

        // Set up params
        SgdParams p;
        p.nEpochs = epochCount;
        p.fullAsync = async;
        faasm::writeParamsToState(PARAMS_KEY, p);

        // Finish a set number of epochs
        faasm::initCounter(EPOCH_COUNT_KEY, async);
        for (int i = 0; i < finishedCount; i++) {
            faasm::incrementCounter(EPOCH_COUNT_KEY, async);
        }

        // Check finished report
        const std::string finishedResult = execStrFunction("sgd_finished");
        if (expectFinished) {
            REQUIRE(finishedResult == "true");
        } else {
            REQUIRE(finishedResult == "false");
        }
    }
}