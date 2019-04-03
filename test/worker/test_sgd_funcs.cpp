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
        resetEmulatorUser();
    }

    std::string execStrFunction(const std::string &funcName) {
        message::Message call;
        call.set_user("sgd");
        call.set_function(funcName);
        call.set_resultkey("foobar");

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
        FaasmMemory mem;
        SgdParams p;
        p.nEpochs = 5;
        p.fullAsync = async;
        faasm::writeParamsToState(&mem, PARAMS_KEY, p);

        // Set up memory
        std::vector<double> losses = {1000.5, 900.22, 20.1, 5.5, 99.999};
        std::vector<double> lossTimestamps = {10.0, 10.0 + 1.23, 10.0 + 12.34, 10.0 + 123.456, 10.0 + 1234.56};
        size_t nBytes = p.nEpochs * sizeof(double);
        mem.writeState(LOSSES_KEY, reinterpret_cast<uint8_t *>(losses.data()), nBytes, async);
        mem.writeState(LOSS_TIMESTAMPS_KEY, reinterpret_cast<uint8_t *>(lossTimestamps.data()), nBytes, async);

        // Call function
        std::string output = execStrFunction("sgd_loss");

        // Expected is relative timestamp - loss
        std::string expct = "0.0 - 1000.500, 1.230 - 900.219, 12.340 - 20.100, 123.456 - 5.500, 1234.560 - 99.999, ";
        REQUIRE(output == expct);

        tearDown();
    }

    TEST_CASE("Test sgd barrier", "[worker]") {
        bool async;
        SECTION("Synchronous") {
            async = false;
        }
        SECTION("Asynchronous") {
            async = true;
        }

        setUp(async);

        // Set up params
        FaasmMemory mem;
        SgdParams p;
        p.nEpochs = 10;
        p.nBatches = 3;
        p.fullAsync = async;
        p.nTrain = 100;
        p.learningRate = 0.1;
        p.learningDecay = 0.8;
        faasm::writeParamsToState(&mem, PARAMS_KEY, p);

        // Zero errors and losses
        faasm::zeroLosses(&mem, p);
        faasm::zeroErrors(&mem, p);

        // Set the epoch count
        faasm::initCounter(&mem, EPOCH_COUNT_KEY, async);
        faasm::incrementCounter(&mem, EPOCH_COUNT_KEY, async);
        faasm::incrementCounter(&mem, EPOCH_COUNT_KEY, async);
        REQUIRE(faasm::getCounter(&mem, EPOCH_COUNT_KEY, async) == 2);
        REQUIRE(!readEpochFinished(&mem, p));

        // Set all as finished and set an error
        long totalErrorBytes = p.nBatches * sizeof(double);
        double totalError = 0;
        for (int i = 0; i < p.nBatches; i++) {
            faasm::writeFinishedFlag(&mem, p, i);

            double error = i * 1.5;
            totalError += error;
            long offset = i * sizeof(double);
            mem.writeStateOffset(ERRORS_KEY, totalErrorBytes, offset, reinterpret_cast<uint8_t *>(&error),
                                 sizeof(double), async);
        }

        // Check the mean squared error comes out correctly
        double expectedLoss = std::sqrt(totalError) / std::sqrt(p.nTrain);
        REQUIRE(faasm::readRootMeanSquaredError(&mem, p) == expectedLoss);

        // Execute
        execStrFunction("sgd_barrier");
        
        // Check counter incremented and reported as finished
        REQUIRE(readEpochFinished(&mem, p));
        REQUIRE(faasm::getCounter(&mem, EPOCH_COUNT_KEY, async) == 3);

        // Check losses and loss timestamps are written
        size_t nBytes = p.nEpochs * sizeof(double);
        int offset = 2 * sizeof(double);
        uint8_t *lossBytes = mem.readStateOffset(LOSSES_KEY, nBytes, offset, sizeof(double), async);
        uint8_t *lossTsBytes = mem.readStateOffset(LOSS_TIMESTAMPS_KEY, nBytes, offset, sizeof(double), async);

        double loss = *reinterpret_cast<double *>(lossBytes);
        REQUIRE(loss == expectedLoss);

        double lossTs = *reinterpret_cast<double *>(lossTsBytes);
        REQUIRE(lossTs > 0);

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
        FaasmMemory mem;
        SgdParams p;
        p.nEpochs = epochCount;
        p.fullAsync = async;
        faasm::writeParamsToState(&mem, PARAMS_KEY, p);

        // Finish a set number of epochs
        faasm::initCounter(&mem, EPOCH_COUNT_KEY, async);
        for (int i = 0; i < finishedCount; i++) {
            faasm::incrementCounter(&mem, EPOCH_COUNT_KEY, async);
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