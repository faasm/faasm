#include <catch/catch.hpp>

#include "utils.h"

#include <faasm/sgd.h>
#include <faasm/matrix.h>

#include <redis/Redis.h>
#include <util/environment.h>
#include <emulator/emulator.h>
#include <util/state.h>


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
        cleanSystem();

        SgdParams params = getDummySgdParams();

        const char *key = "params_test";

        // Write to state
        writeParamsToState(key, params, true);

        // Read back and check
        SgdParams actual = readParamsFromState(key, true);
        checkSgdParamEquality(actual, params);
    }

    TEST_CASE("Test setting up dummy data", "[sgd]") {
        cleanSystem();

        // Set up the problem
        SgdParams params = getDummySgdParams();
        setUpDummyProblem(params);

        // Check params are set up
        SgdParams actual = readParamsFromState(PARAMS_KEY, true);
        checkSgdParamEquality(actual, params);

        // Check weights
        const MatrixXd actualWeights = readMatrixFromState(WEIGHTS_KEY, 1, params.nWeights, true);
        REQUIRE(actualWeights.rows() == 1);
        REQUIRE(actualWeights.cols() == params.nWeights);

        const MatrixXd actualOutputs = readMatrixFromState(OUTPUTS_KEY, params.nWeights, params.nTrain, true);
        REQUIRE(actualOutputs.rows() == params.nWeights);
        REQUIRE(actualOutputs.cols() == params.nTrain);
    }

    TEST_CASE("Test hinge loss updates", "[sgd]") {
        cleanSystem();

        int nWeights = 4;
        SgdParams params;
        params.nWeights = nWeights;
        params.learningRate = 0.1;
        params.nBatches = 1;
        params.batchSize = 2;
        params.syncInterval = 100;

        // Dummy initial weights
        MatrixXd weights(1, nWeights);
        weights << 1, 2, 3, 4;

        // Persist weights to allow updates
        writeMatrixToState(WEIGHTS_KEY, weights, true);

        // Set up some dummy feature counts
        std::vector<int> featureCounts(4);
        std::fill(featureCounts.begin(), featureCounts.end(), 1);
        uint8_t *featureBytes = BYTES(featureCounts.data());
        faasmWriteState(FEATURE_COUNTS_KEY, featureBytes, 4 * sizeof(int));
        faasmPushState(FEATURE_COUNTS_KEY);

        // Copy of weights for testing
        MatrixXd weightsCopy = weights;

        // Fake up sparse inputs with all permutations
        SparseMatrix<double> inputs(nWeights, 2);
        std::vector<Triplet<double>> tripletList;

        // Inputs to look like this:
        // -3.2  3.1
        // 0     -1.5
        // 2.9   0
        // 0     0
        tripletList.emplace_back(Triplet<double>(0, 0, -3.2));
        tripletList.emplace_back(Triplet<double>(0, 1, 3.1));
        tripletList.emplace_back(Triplet<double>(1, 1, -1.5));
        tripletList.emplace_back(Triplet<double>(2, 0, 2.9));

        // Set up inputs in state
        inputs.setFromTriplets(tripletList.begin(), tripletList.end());
        faasm::writeSparseMatrixToState(INPUTS_KEY, inputs, true);

        // Check what the predictions are pre-update
        MatrixXd preUpdate = weights * inputs;

        // Now run the actual updates and check the impact
        int startIdx = 0;
        int endIdx = 2;

        // Classification-style outputs
        MatrixXd outputs(1, 2);
        outputs << -1, 1;

        faasm::writeMatrixToState(OUTPUTS_KEY, outputs, true);

        hingeLossWeightUpdate(params, startIdx, endIdx);

        // Ensure everything pushed
        faasmPushStatePartial(WEIGHTS_KEY);

        // Check weights have been updated where necessary
        const MatrixXd actualWeights = readMatrixFromState(WEIGHTS_KEY, 1, nWeights, true);
        REQUIRE(actualWeights.rows() == 1);
        REQUIRE(actualWeights.cols() == nWeights);

        // Where there are input values, weights should be updated
        REQUIRE(actualWeights(0, 0) != weightsCopy(0, 0));
        REQUIRE(actualWeights(0, 1) != weightsCopy(0, 1));
        REQUIRE(actualWeights(0, 2) != weightsCopy(0, 2));

        // Where no input values, weights should remain the same
        REQUIRE(actualWeights(0, 3) == weightsCopy(0, 3));
    }

    void checkAppendOnlyInState(const std::string &user, const char *key, long nDoubles,
                                const std::vector<double> &expected) {
        size_t bufferSize = nDoubles * sizeof(double);
        std::vector<uint8_t> actualBytes(bufferSize, 0);

        const std::shared_ptr<state::StateKeyValue> &kv = state::getGlobalState().getKV(user, key);
        kv->getAppended(actualBytes.data(), bufferSize, nDoubles);
        REQUIRE(!actualBytes.empty());

        auto actualPtr = reinterpret_cast<double *>(actualBytes.data());
        std::vector<double> actual(actualPtr, actualPtr + expected.size());

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test writing errors to state", "[sgd]") {
        cleanSystem();

        MatrixXd a = randomDenseMatrix(1, 5);
        MatrixXd b = randomDenseMatrix(1, 5);
        MatrixXd c = randomDenseMatrix(1, 5);
        MatrixXd d = randomDenseMatrix(1, 5);

        SgdParams params = getDummySgdParams();
        params.nBatches = 4;

        // Check zeroing out errors
        faasmClearAppendedState(ERRORS_KEY);

        // Work out expectation
        double expected1 = calculateHingeError(a, b);
        double expected2 = calculateHingeError(a, b);

        // Write and check
        writeHingeError(params, a, b);
        writeHingeError(params, a, b);
        checkAppendOnlyInState(getEmulatorUser(), ERRORS_KEY, 2, {expected1, expected2});
    }

    TEST_CASE("Test reading errors from state", "[sgd]") {
        cleanSystem();
        const std::string user = getEmulatorUser();

        SgdParams p = getDummySgdParams();
        p.nBatches = 3;
        p.nTrain = 20;

        // Write the error for three batches
        MatrixXd a = randomDenseMatrix(1, 5);
        MatrixXd b = randomDenseMatrix(1, 5);
        double expected = calculateHingeError(a, b);

        // Write errors
        writeHingeError(p, a, b);
        writeHingeError(p, a, b);
        writeHingeError(p, a, b);

        // Check
        std::vector<double> expectedStateB = {expected, expected, expected};
        checkAppendOnlyInState(user, ERRORS_KEY, 3, expectedStateB);

        // Work out what the result should be
        double expectedRmse = sqrt((3 * expected) / p.nTrain);
        double actual = faasm::readRootMeanSquaredError(p);
        REQUIRE(abs(actual - expectedRmse) < 0.0000001);
    }
}