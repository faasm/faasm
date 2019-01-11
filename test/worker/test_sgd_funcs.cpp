#include <catch/catch.hpp>

#include <emulator/emulator.h>
#include <worker/worker.h>

#include "utils.h"
#include <faasm/sgd.h>
#include <faasm/counter.h>


using namespace worker;

namespace tests {
    static void setUp() {
        cleanSystem();

        state::getGlobalState().forceClearAll();

        setEmulatorUser("sgd");
    }

    static void tearDown() {
        resetEmulatorUser();
    }

    std::string execFunction(const std::string &funcName) {
        message::Message call;
        call.set_user("sgd");
        call.set_function(funcName);
        call.set_resultkey("foobar");

        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
        REQUIRE(w.isInitialised());
        REQUIRE(!w.isBound());

        // Call the function
        infra::Scheduler::callFunction(call);

        // Process the bind and execute messages
        w.processNextMessage();
        w.processNextMessage();

        infra::Redis &redisQueue = infra::Redis::getQueue();
        const message::Message result = redisQueue.getFunctionResult(call);
        return result.outputdata();
    }

    void checkSgdLosses(bool async) {
        setUp();

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
        std::string output = execFunction("sgd_loss");

        // Expected is relative timestamp - loss
        std::string expct = "0.00 - 1000.5000, 1.23 - 900.2200, 12.34 - 20.1000, 123.46 - 5.5000, 1234.56 - 99.9990, ";
        REQUIRE(output == expct);

        tearDown();
    }

    TEST_CASE("Test sgd losses", "[worker]") {
        checkSgdLosses(false);
    }

    TEST_CASE("Test sgd losses async", "[worker]") {
        checkSgdLosses(true);
    }

    void checkSgdBarrier(bool async) {
        setUp();

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

        // Double check it's registering as finished
        REQUIRE(faasm::readEpochFinished(&mem, p));

        // Execute
        execFunction("sgd_barrier");

        // Check losses and loss timestamps are written
        size_t nBytes = p.nEpochs * sizeof(double);
        int offset = 2 * sizeof(double);
        uint8_t *lossBytes = mem.readStateOffset(LOSSES_KEY, nBytes, offset, sizeof(double), async);
        uint8_t *lossTsBytes = mem.readStateOffset(LOSS_TIMESTAMPS_KEY, nBytes, offset, sizeof(double), async);

        double loss = *reinterpret_cast<double *>(lossBytes);
        double expectedLoss = std::sqrt(totalError) / std::sqrt(p.nTrain);
        REQUIRE(loss == expectedLoss);

        double lossTs = *reinterpret_cast<double *>(lossTsBytes);
        REQUIRE(lossTs > 0);

        // Check learning rate has decayed
//        const SgdParams paramsAfter = faasm::readParamsFromState(&mem, PARAMS_KEY, async);
//        REQUIRE(paramsAfter.learningRate == 0.8 * 0.1);

        tearDown();
    }

    TEST_CASE("Test sgd barrier", "[worker]") {
        checkSgdBarrier(false);
    }

    TEST_CASE("Test sgd barrier async", "[worker]") {
        checkSgdBarrier(true);
    }
}