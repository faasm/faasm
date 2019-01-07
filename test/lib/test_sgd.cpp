#include <catch/catch.hpp>

#include "utils.h"

#include <faasm/sgd.h>
#include <faasm/matrix.h>

#include <data/data.h>
#include <infra/infra.h>
#include <state/state.h>

using namespace faasm;

namespace tests {

    SgdParams getDummySgdParams() {
        SgdParams params;
        params.nTrain = 10;
        params.batchSize = 2;
        params.nBatches = 5;
        params.nWeights = 3;
        params.learningRate = 0.1;
        params.nEpochs = 2;
        params.fullAsync = false;

        return params;
    }

    void checkSgdParamEquality(SgdParams &a, SgdParams &b) {
        REQUIRE(a.nBatches == b.nBatches);
        REQUIRE(a.nWeights == b.nWeights);
        REQUIRE(a.nTrain == b.nTrain);
        REQUIRE(a.learningRate == b.learningRate);
        REQUIRE(a.nEpochs == b.nEpochs);
    }

    TEST_CASE("Test serialising params round trip", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        SgdParams params = getDummySgdParams();

        const char *key = "params_test";
        FaasmMemory mem;

        // Write to state
        writeParamsToState(&mem, key, params);

        // Read back and check
        SgdParams actual = readParamsFromState(&mem, key);
        checkSgdParamEquality(actual, params);
    }

    TEST_CASE("Test setting up dummy data", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        SgdParams params = getDummySgdParams();
        params.fullAsync = false;

        // Set up the problem
        FaasmMemory mem;
        setUpDummyProblem(&mem, params);

        // Check params are set up
        SgdParams actual = readParamsFromState(&mem, PARAMS_KEY);
        checkSgdParamEquality(actual, params);

        // Check weights
        const MatrixXd actualWeights = readMatrixFromState(&mem, WEIGHTS_KEY, 1, params.nWeights);
        REQUIRE(actualWeights.rows() == 1);
        REQUIRE(actualWeights.cols() == params.nWeights);

        const MatrixXd actualOutputs = readMatrixFromState(&mem, OUTPUTS_KEY, params.nWeights, params.nTrain);
        REQUIRE(actualOutputs.rows() == params.nWeights);
        REQUIRE(actualOutputs.cols() == params.nTrain);
    }

    void checkLossUpdates(LossType lossType, bool async) {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        FaasmMemory mem;

        int nWeights = 4;
        SgdParams params;
        params.lossType = lossType;
        params.nWeights = nWeights;
        params.learningRate = 0.1;
        params.nBatches = 1;
        params.batchSize = 2;

        // Dummy initial weights
        MatrixXd weights(1, nWeights);
        weights << 1, 2, 3, 4;

        // Persist weights to allow updates
        writeMatrixToState(&mem, WEIGHTS_KEY, weights, async);

        // Set up some dummy feature counts
        std::vector<int> featureCounts(4);
        std::fill(featureCounts.begin(), featureCounts.end(), 1);
        uint8_t *featureBytes = reinterpret_cast<uint8_t *>(featureCounts.data());
        mem.writeState(FEATURE_COUNTS_KEY, featureBytes, 4 * sizeof(int), async);

        // Copy of weights for testing
        MatrixXd weightsCopy = weights;

        // Fake up sparse inputs with all permutations
        SparseMatrix<double> inputs(nWeights, 2);
        std::vector<Triplet<double>> tripletList;

        if (lossType == RMSE) {
            // Inputs to look like this:
            // 3 3
            // 0 1
            // 2 0
            // 0 0
            tripletList.emplace_back(Triplet<double>(0, 0, 3));
            tripletList.emplace_back(Triplet<double>(0, 1, 3));
            tripletList.emplace_back(Triplet<double>(1, 1, 1));
            tripletList.emplace_back(Triplet<double>(2, 0, 2));
        } else if (lossType == HINGE) {
            // Inputs to look like this:
            // -3.2  3.1
            // 0     -1.5
            // 2.9   0
            // 0     0
            tripletList.emplace_back(Triplet<double>(0, 0, -3.2));
            tripletList.emplace_back(Triplet<double>(0, 1, 3.1));
            tripletList.emplace_back(Triplet<double>(1, 1, -1.5));
            tripletList.emplace_back(Triplet<double>(2, 0, 2.9));
        }

        // Set up inputs in state
        inputs.setFromTriplets(tripletList.begin(), tripletList.end());
        faasm::writeSparseMatrixToState(&mem, INPUTS_KEY, inputs, async);

        // Check what the predictions are pre-update
        MatrixXd preUpdate = weights * inputs;

        // Now run the actual updates and check the impact
        int batchNumber = 0;
        int startIdx = 0;
        int endIdx = 2;
        if (lossType == RMSE) {
            MatrixXd outputs(1, 2);
            outputs << 10, 11;

            faasm::writeMatrixToState(&mem, OUTPUTS_KEY, outputs, async);
            leastSquaresWeightUpdate(&mem, params, batchNumber, startIdx, endIdx);
        } else if (lossType == HINGE) {
            // Classification-style outputs
            MatrixXd outputs(1, 2);
            outputs << -1, 1;

            faasm::writeMatrixToState(&mem, OUTPUTS_KEY, outputs, async);

            int epoch = 3;
            hingeLossWeightUpdate(&mem, params, epoch, batchNumber, startIdx, endIdx);
        }

        // Ensure everything pushed
        mem.pushStatePartial(WEIGHTS_KEY);

        // Check weights have been updated where necessary
        const MatrixXd actualWeights = readMatrixFromState(&mem, WEIGHTS_KEY, 1, nWeights, async);
        REQUIRE(actualWeights.rows() == 1);
        REQUIRE(actualWeights.cols() == nWeights);

        // Where there are input values, weights should be updated
        REQUIRE(actualWeights(0, 0) != weightsCopy(0, 0));
        REQUIRE(actualWeights(0, 1) != weightsCopy(0, 1));
        REQUIRE(actualWeights(0, 2) != weightsCopy(0, 2));

        // Where no input values, weights should remain the same
        REQUIRE(actualWeights(0, 3) == weightsCopy(0, 3));
    }

    TEST_CASE("Test least squares updates", "[sgd]") {
        checkLossUpdates(RMSE, false);
    }

    TEST_CASE("Test least squares updates async", "[sgd]") {
        checkLossUpdates(RMSE, true);
    }

    TEST_CASE("Test hinge loss updates", "[sgd]") {
        checkLossUpdates(HINGE, false);
    }

    TEST_CASE("Test hinge loss updates async", "[sgd]") {
        checkLossUpdates(HINGE, true);
    }

    void checkDoubleArrayInState(infra::Redis &r, const char *key, std::vector<double> expected) {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        std::string actualKey("demo_" + std::string(key));
        std::vector<uint8_t> actualBytes = redisQueue.get(actualKey);

        auto actualPtr = reinterpret_cast<double *>(actualBytes.data());
        std::vector<double> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    void checkIntArrayInState(infra::Redis &r, const char *key, std::vector<int> expected) {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        std::string actualKey("demo_" + std::string(key));
        std::vector<uint8_t> actualBytes = redisQueue.get(actualKey);

        auto actualPtr = reinterpret_cast<int *>(actualBytes.data());
        std::vector<int> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test writing errors to state", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        MatrixXd a = randomDenseMatrix(1, 5);
        MatrixXd b = randomDenseMatrix(1, 5);
        MatrixXd c = randomDenseMatrix(1, 5);
        MatrixXd d = randomDenseMatrix(1, 5);

        // Check no errors set initially
        const std::vector<uint8_t> initial = redisQueue.get(ERRORS_KEY);
        REQUIRE(initial.empty());

        FaasmMemory memory;
        SgdParams params = getDummySgdParams();
        params.nBatches = 4;

        // Check zeroing out errors
        zeroErrors(&memory, params);
        checkDoubleArrayInState(redisQueue, ERRORS_KEY, {0, 0, 0, 0});

        // Work out expectation
        double expected1 = calculateSquaredError(a, b);
        double expected2 = calculateSquaredError(a, b);

        // Write errors to memory
        writeSquaredError(&memory, params, 0, a, b);
        writeSquaredError(&memory, params, 2, a, b);

        checkDoubleArrayInState(redisQueue, ERRORS_KEY, {expected1, 0, expected2, 0});
    }

    TEST_CASE("Test reading errors from state", "[sgd]") {
        infra::Redis &redisState = infra::Redis::getState();
        redisState.flushAll();

        state::getGlobalState().forceClearAll();

        FaasmMemory memory;
        SgdParams p = getDummySgdParams();
        p.nBatches = 3;
        p.nTrain = 20;

        // With nothing set up, error should be zero
        zeroErrors(&memory, p);
        double initial = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(initial == 0);

        // Write the error for two of the three batches
        MatrixXd a = randomDenseMatrix(1, 5);
        MatrixXd b = randomDenseMatrix(1, 5);
        double expected = calculateSquaredError(a, b);

        writeSquaredError(&memory, p, 0, a, b);
        writeSquaredError(&memory, p, 1, a, b);

        // Check these have been written
        checkDoubleArrayInState(redisState, ERRORS_KEY, {expected, expected, 0});

        // Error should just include the 2 written
        double expectedRmse1 = sqrt((2 * expected) / p.nTrain);
        double actual1 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(actual1 == expectedRmse1);

        // Now write error for a third batch
        writeSquaredError(&memory, p, 2, a, b);
        checkDoubleArrayInState(redisState, ERRORS_KEY, {expected, expected, expected});

        // Work out what the result should be
        double expectedRmse2 = sqrt((3 * expected) / p.nTrain);
        double actual2 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(abs(actual2 - expectedRmse2) < 0.0000001);
    }

    TEST_CASE("Test zeroing losses", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();

        state::getGlobalState().forceClearAll();

        SgdParams p = getDummySgdParams();
        p.nBatches = 10;
        p.nEpochs = 5;

        FaasmMemory mem;

        // Zero and check it's worked
        zeroLosses(&mem, p);
        checkDoubleArrayInState(redisQueue, LOSSES_KEY, {0, 0, 0, 0, 0});
        checkDoubleArrayInState(redisQueue, LOSS_TIMESTAMPS_KEY, {0, 0, 0, 0, 0});

        // Update with some other values
        std::vector<double> losses = {2.2, 3.3, 4.4, 5.5, 0.0};
        auto lossBytes = reinterpret_cast<uint8_t *>(losses.data());
        mem.writeState(LOSSES_KEY, lossBytes, 5 * sizeof(double));

        std::vector<double> timestamps = {100.0, 200.2, 300.3, 1000.1, 2000.2};
        auto timestampBytes = reinterpret_cast<uint8_t *>(timestamps.data());
        mem.writeState(LOSS_TIMESTAMPS_KEY, timestampBytes, 5 * sizeof(double));

        checkDoubleArrayInState(redisQueue, LOSSES_KEY, losses);
        checkDoubleArrayInState(redisQueue, LOSS_TIMESTAMPS_KEY, timestamps);

        // Zero again and check it's worked
        zeroLosses(&mem, p);
        checkDoubleArrayInState(redisQueue, LOSSES_KEY, {0, 0, 0, 0, 0});
        checkDoubleArrayInState(redisQueue, LOSS_TIMESTAMPS_KEY, {0, 0, 0, 0, 0});
    }

    TEST_CASE("Test setting finished flags", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        SgdParams p = getDummySgdParams();
        p.nBatches = 3;

        FaasmMemory mem;

        zeroFinished(&mem, p);
        REQUIRE(!readEpochFinished(&mem, p));

        writeFinishedFlag(&mem, p, 0);
        writeFinishedFlag(&mem, p, 2);
        REQUIRE(!readEpochFinished(&mem, p));
        checkIntArrayInState(redisQueue, FINISHED_KEY, {1, 0, 1});

        writeFinishedFlag(&mem, p, 1);
        checkIntArrayInState(redisQueue, FINISHED_KEY, {1, 1, 1});
        REQUIRE(readEpochFinished(&mem, p));
    }

    TEST_CASE("Test zeroing finished flags", "[sgd]") {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();

        state::getGlobalState().forceClearAll();

        SgdParams p = getDummySgdParams();
        p.nBatches = 3;

        FaasmMemory mem;

        // Zero and check it's worked
        zeroFinished(&mem, p);
        checkIntArrayInState(redisQueue, FINISHED_KEY, {0, 0, 0});

        // Update with some other values
        std::vector<int> finished = {1, 0, 1};
        auto lossBytes = reinterpret_cast<uint8_t *>(finished.data());
        mem.writeState(FINISHED_KEY, lossBytes, 5 * sizeof(int));

        checkIntArrayInState(redisQueue, FINISHED_KEY, finished);

        // Zero again and check it's worked
        zeroFinished(&mem, p);
        checkIntArrayInState(redisQueue, FINISHED_KEY, {0, 0, 0});
    }

}