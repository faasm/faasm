#include <catch/catch.hpp>

#include "utils.h"

#include <faasm/sgd.h>
#include <faasm/matrix.h>

#include <infra/infra.h>

using namespace faasm;

namespace tests {

    SgdParams getDummySgdParams() {
        SgdParams params;
        params.nBatches = 4;
        params.nWeights = 3;
        params.nTrain = 10;
        params.learningRate = 0.1;
        params.nEpochs = 2;

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
        redisQueue.flushAll();

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
        redisQueue.flushAll();

        SgdParams params = getDummySgdParams();

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

    void checkLossUpdates(LossType lossType) {
        redisQueue.flushAll();

        FaasmMemory mem;

        int nWeights = 4;
        SgdParams params;
        params.lossType = lossType;
        params.nWeights = nWeights;
        params.learningRate = 0.1;

        // Dummy initial weights
        MatrixXd weights(1, nWeights);
        weights << 1, 2, 3, 4;

        // Persist weights to allow updates
        writeMatrixToState(&mem, WEIGHTS_KEY, weights);

        // Copy of weights for testing
        MatrixXd weightsCopy = weights;

        // Fake up sparse inputs with all permutations
        SparseMatrix<double> inputs(nWeights, 2);
        std::vector<Triplet<double>> tripletList;

        if(lossType == RMSE) {
            // Inputs to look like this:
            // 3 3
            // 0 1
            // 2 0
            // 0 0
            tripletList.emplace_back(Triplet<double>(0, 0, 3));
            tripletList.emplace_back(Triplet<double>(0, 1, 3));
            tripletList.emplace_back(Triplet<double>(1, 1, 1));
            tripletList.emplace_back(Triplet<double>(2, 0, 2));
        }
        else if(lossType == HINGE) {
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

        inputs.setFromTriplets(tripletList.begin(), tripletList.end());

        // Check what the predictions are pre-update
        MatrixXd preUpdate = weights * inputs;

        // Now run the actual updates and check the impact
        MatrixXd postUpdate;
        if(lossType == RMSE) {
            MatrixXd outputs(1, 2);
            outputs << 10, 11;

            postUpdate = leastSquaresWeightUpdate(&mem, params, inputs, outputs);
        }
        else if(lossType == HINGE) {
            // Classification-style outputs
            MatrixXd outputs(1, 2);
            outputs << -1, 1;

            int epoch = 3;
            postUpdate = hingeLossWeightUpdate(&mem, params, epoch, inputs, outputs);
        }

        // Check the post-update values are different but same shape
        REQUIRE(postUpdate.rows() == preUpdate.rows());
        REQUIRE(postUpdate.cols() == preUpdate.cols());
        REQUIRE(postUpdate != preUpdate);

        // Check weights have been updated where necessary
        const MatrixXd actualWeights = readMatrixFromState(&mem, WEIGHTS_KEY, 1, nWeights);
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
        checkLossUpdates(RMSE);
    }

    TEST_CASE("Test hinge loss updates", "[sgd]") {
        checkLossUpdates(HINGE);
    }

    TEST_CASE("Test SGD with least squares converges", "[sgd]") {
        redisQueue.flushAll();

        // Perform minibatch
        SgdParams params;
        params.lossType = RMSE;
        params.nBatches = 2500;
        params.nWeights = 4;
        params.nTrain = 5000;
        params.learningRate = 0.01;
        params.nEpochs = 10;

        // Set up the problem
        FaasmMemory mem;
        setUpDummyProblem(&mem, params);

        SparseMatrix<double> inputs = readSparseMatrixFromState(&mem, INPUTS_KEY);
        MatrixXd outputs = readMatrixFromState(&mem, OUTPUTS_KEY, 1, params.nTrain);

        // Work out the error before we start
        MatrixXd weights = readMatrixFromState(&mem, WEIGHTS_KEY, 1, params.nWeights);
        MatrixXd initialOutput = weights * inputs;
        double startingLoss = calculateRootMeanSquaredError(initialOutput, outputs);

        // Run multiple updates
        double finalLoss = 0;
        for (int e = 0; e < params.nEpochs; e++) {
            finalLoss = doSgdStep(&mem, params, e, inputs, outputs);
        }

        REQUIRE(finalLoss < startingLoss);
    }

    void checkDoubleArrayInState(infra::Redis &r, const char *key, std::vector<double> expected) {
        std::vector<uint8_t> actualBytes = redisQueue.get(key);

        auto actualPtr = reinterpret_cast<double *>(actualBytes.data());
        std::vector<double> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    void checkIntArrayInState(infra::Redis &r, const char *key, std::vector<int> expected) {
        std::vector<uint8_t> actualBytes = redisQueue.get(key);

        auto actualPtr = reinterpret_cast<int *>(actualBytes.data());
        std::vector<int> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test writing errors to state", "[sgd]") {
        redisQueue.flushAll();

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
        writeSquaredError(&memory, 0, a, b);
        writeSquaredError(&memory, 2, a, b);

        checkDoubleArrayInState(redisQueue, ERRORS_KEY, {expected1, 0, expected2, 0});
    }

    TEST_CASE("Test reading errors from state", "[sgd]") {
        redisQueue.flushAll();

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

        writeSquaredError(&memory, 0, a, b);
        writeSquaredError(&memory, 1, a, b);

        // Check these have been written
        checkDoubleArrayInState(redisQueue, ERRORS_KEY,{expected, expected, 0});

        // Error should just include the 2 written
        double expectedRmse1 = sqrt((2 * expected) / p.nTrain);
        double actual1 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(actual1 == expectedRmse1);

        // Now write error for a third batch
        writeSquaredError(&memory, 2, a, b);
        checkDoubleArrayInState(redisQueue, ERRORS_KEY,{expected, expected, expected});

        // Work out what the result should be
        double expectedRmse2 = sqrt((3 * expected) / p.nTrain);
        double actual2 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(abs(actual2 - expectedRmse2) < 0.0000001);
    }

    TEST_CASE("Test zeroing losses", "[sgd]") {
        redisQueue.flushAll();

        SgdParams p = getDummySgdParams();
        p.nBatches = 10;
        p.nEpochs = 5;

        FaasmMemory mem;

        // Zero and check it's worked
        zeroLosses(&mem, p);
        checkDoubleArrayInState(redisQueue, LOSSES_KEY, {0, 0, 0, 0, 0});

        // Update with some other values
        std::vector<double> losses = {2.2, 3.3, 4.4, 5.5, 0.0};
        auto lossBytes = reinterpret_cast<uint8_t *>(losses.data());
        mem.writeState(LOSSES_KEY, lossBytes, 5 * sizeof(double));

        checkDoubleArrayInState(redisQueue, LOSSES_KEY, losses);

        // Zero again and check it's worked
        zeroLosses(&mem, p);
        checkDoubleArrayInState(redisQueue, LOSSES_KEY, {0, 0, 0, 0, 0});
    }

    TEST_CASE("Test setting finished flags", "[sgd]") {
        redisQueue.flushAll();

        SgdParams p = getDummySgdParams();
        p.nBatches = 3;

        FaasmMemory mem;

        zeroFinished(&mem, p);
        REQUIRE(!readEpochFinished(&mem, p));

        writeFinishedFlag(&mem, 0);
        writeFinishedFlag(&mem, 2);
        REQUIRE(!readEpochFinished(&mem, p));
        checkIntArrayInState(redisQueue, FINISHED_KEY, {1, 0, 1});

        writeFinishedFlag(&mem, 1);
        checkIntArrayInState(redisQueue, FINISHED_KEY, {1, 1, 1});
        REQUIRE(readEpochFinished(&mem, p));
    }

    TEST_CASE("Test zeroing finished flags", "[sgd]") {
        redisQueue.flushAll();

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