#include <catch/catch.hpp>

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
        params.maxEpochs = 2;

        return params;
    }

    void checkSgdParamEquality(SgdParams &a, SgdParams &b) {
        REQUIRE(a.nBatches == b.nBatches);
        REQUIRE(a.nWeights == b.nWeights);
        REQUIRE(a.nTrain == b.nTrain);
        REQUIRE(a.learningRate == b.learningRate);
        REQUIRE(a.maxEpochs == b.maxEpochs);
    }

    TEST_CASE("Test serialising params round trip", "[sgd]") {
        infra::Redis r;
        r.flushAll();

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
        infra::Redis r;
        r.flushAll();

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

    TEST_CASE("Test least squares updates", "[sgd]") {
        infra::Redis r;
        r.flushAll();

        FaasmMemory mem;

        int nWeights = 4;
        SgdParams params;
        params.nWeights = nWeights;
        params.learningRate = 0.1;

        // Dummy initial weights
        MatrixXd weights(1, nWeights);
        weights << 1, 2, 3, 4;

        // Persist weights to allow updates
        writeMatrixState(&mem, WEIGHTS_KEY, weights);

        // Copy of weights for testing
        MatrixXd weightsCopy = weights;

        // Fake up sparse inputs with all permutations
        MatrixXd inputs(nWeights, 2);
        inputs << 3, 3,
                0, 1,
                2, 0,
                0, 0;

        // Outputs
        MatrixXd outputs(1, 2);
        outputs << 10, 11;

        const MatrixXd &actualOutputs = leastSquaresWeightUpdate(&mem, params, weights, inputs, outputs);

        // Check actuals are as they should be
        MatrixXd expected(1, 2);
        expected << 9, 5;

        REQUIRE(expected == actualOutputs);

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

    double doSgdStep(FaasmMemory *mem, SgdParams &params, MatrixXd &inputs, MatrixXd &outputs) {
        // Shuffle inputs
        faasm::shufflePairedMatrixColumns(inputs, outputs);

        // Prepare update loop
        int batchSize = params.nTrain / params.nBatches;
        long startCol = 0;
        MatrixXd weights = readMatrixFromState(mem, WEIGHTS_KEY, 1, params.nWeights);

        // Perform batch updates to weights
        for (int b = 0; b < params.nBatches; b++) {
            MatrixXd inputBatch = inputs.block(0, startCol, params.nWeights, batchSize);
            MatrixXd outputBatch = outputs.block(0, startCol, 1, batchSize);

            // Perform the update
            leastSquaresWeightUpdate(mem, params, weights, inputBatch, outputBatch);

            // Update parameters
            startCol += batchSize;
            weights = readMatrixFromState(mem, WEIGHTS_KEY, 1, params.nWeights);
        }

        // Calculate the actual values
        MatrixXd actual = weights * inputs;

        double rmse = calculateRootMeanSquaredError(actual, outputs);
        return rmse;
    }

    TEST_CASE("Test SGD with least squares converges", "[sgd]") {
        infra::Redis r;
        r.flushAll();

        // Perform "proper" SGD with batch size of 1
        SgdParams params;
        params.nBatches = 5000;
        params.nWeights = 4;
        params.nTrain = 5000;
        params.learningRate = 0.01;
        params.maxEpochs = 10;

        // Set up the problem
        FaasmMemory mem;
        setUpDummyProblem(&mem, params);

        MatrixXd inputs = readMatrixFromState(&mem, INPUTS_KEY, params.nWeights, params.nTrain);
        MatrixXd outputs = readMatrixFromState(&mem, OUTPUTS_KEY, 1, params.nTrain);

        // Work out the error before we start
        MatrixXd weights = readMatrixFromState(&mem, WEIGHTS_KEY, 1, params.nWeights);
        MatrixXd initialOutput = weights * inputs;
        double startingLoss = calculateRootMeanSquaredError(initialOutput, outputs);

        // Run multiple updates
        double finalLoss = 0;
        for(int e = 0; e < params.maxEpochs; e++ ) {
            finalLoss = doSgdStep(&mem, params, inputs, outputs);
        }

        REQUIRE(finalLoss < startingLoss);
    }

    void checkErrorsInState(infra::Redis &r, std::vector<double> expected) {
        std::vector<uint8_t> actualBytes = r.get(ERRORS_KEY);

        auto actualPtr = reinterpret_cast<double *>(actualBytes.data());
        std::vector<double> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test writing errors to state", "[sgd]") {
        infra::Redis r;
        r.flushAll();

        MatrixXd a = randomDenseMatrix(1, 5);
        MatrixXd b = randomDenseMatrix(1, 5);
        MatrixXd c = randomDenseMatrix(1, 5);
        MatrixXd d = randomDenseMatrix(1, 5);

        // Check no errors set initially
        const std::vector<uint8_t> initial = r.get(ERRORS_KEY);
        REQUIRE(initial.empty());
        
        FaasmMemory memory;
        SgdParams params = getDummySgdParams();
        params.nBatches = 4;

        // Check zeroing out errors
        zeroErrors(&memory, params);
        checkErrorsInState(r, {0, 0, 0, 0});

        // Work out expectation
        double expected1 = calculateSquaredError(a, b);
        double expected2 = calculateSquaredError(a, b);

        // Write errors to memory
        writeSquaredError(&memory, 0, a, b);
        writeSquaredError(&memory, 2, a, b);

        checkErrorsInState(r, {expected1, 0, expected2, 0});
    }
    
    TEST_CASE("Test reading errors from state", "[sgd]") {
        infra::Redis r;
        r.flushAll();

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
        checkErrorsInState(r, {expected, expected, 0});

        // Overall error should still be zero
        double actual1 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(actual1 == 0);

        // Now write error for a third batch
        writeSquaredError(&memory, 2, a, b);
        checkErrorsInState(r, {expected, expected, expected});

        // Work out what the result should be (note that we're writing the same error for each one)
        double expectedRmse = sqrt((3*expected) / p.nTrain);

        double actual2 = faasm::readRootMeanSquaredError(&memory, p);
        REQUIRE(abs(actual2 - expectedRmse) < 0.0000001);
    }
}
