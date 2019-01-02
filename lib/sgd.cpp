#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
    SgdParams setUpReutersParams(FaasmMemory *memory, int nBatches, int epochs) {
        // Set up reuters params
        SgdParams p;
        p.lossType = HINGE;
        p.nWeights = REUTERS_N_FEATURES;
        p.nTrain = REUTERS_N_EXAMPLES;
        p.learningRate = REUTERS_LEARNING_RATE;
        p.learningDecay = REUTERS_LEARNING_DECAY;
        p.nEpochs = epochs;
        p.mu = 1.0;

        // Round up number of batches
        p.nBatches = nBatches;
        p.batchSize = (REUTERS_N_EXAMPLES + nBatches - 1) / nBatches;

        // Full sync or not
        p.fullAsync = REUTERS_FULL_ASYNC;

        // Write params (will take async/ not from params themselves)
        writeParamsToState(memory, PARAMS_KEY, p);

        return p;
    }

    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params) {
        size_t nBytes = sizeof(SgdParams);
        auto bytePtr = reinterpret_cast<const uint8_t *>(&params);

        // Allow full async
        memory->writeState(keyName, bytePtr, nBytes, params.fullAsync);
    }

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName, bool async) {
        size_t nBytes = sizeof(SgdParams);
        auto buffer = new uint8_t[nBytes];

        memory->readState(keyName, buffer, nBytes, async);

        SgdParams s{};
        memcpy(&s, buffer, nBytes);

        delete[] buffer;
        return s;
    }

    void hingeLossWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, int epoch, int batchNumber,
                               int startIdx, int endIdx) {

        // Load this batch of inputs
        Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY,
                                                                            startIdx, endIdx, true);

        // Load this batch of outputs
        Map<const MatrixXd> outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, sgdParams.nTrain,
                                                           startIdx, endIdx, 1, true);

        // Load the weights
        size_t nWeightBytes = sgdParams.nWeights * sizeof(double);
        uint8_t *weightDataByteBuffer = memory->readState(WEIGHTS_KEY, nWeightBytes, true);

        auto weightDataBuffer = reinterpret_cast<double *>(weightDataByteBuffer);

        // Read in the feature counts (will be constant)
        size_t nFeatureCountBytes = sgdParams.nWeights * sizeof(int);
        uint8_t *featureCountByteBuffer = memory->readState(FEATURE_COUNTS_KEY, nFeatureCountBytes, true);
        auto featureCountBuffer = reinterpret_cast<int *>(featureCountByteBuffer);

        // Shuffle examples in this batch
        int *cols = randomIntRange(inputs.outerSize());

        // Iterate through all training examples (i.e. columns)
        for (int c = 0; c < inputs.outerSize(); ++c) {
            int col = cols[c];

            double thisOutput = outputs.coeff(0, col);

            double thisPrediction = 0.0;
            for (Map<const SparseMatrix<double>>::InnerIterator it(inputs, col); it; ++it) {
                double weight = weightDataBuffer[it.row()];
                thisPrediction += (weight * it.value());
            }
            thisPrediction *= thisOutput;

            double adjustment = sgdParams.learningRate * thisOutput;
            double constScalar = sgdParams.learningRate * sgdParams.mu;

            // Iterate through all non-zero input values in this column and update the relevant weight accordingly
            for (Map<const SparseMatrix<double>>::InnerIterator it(inputs, col); it; ++it) {
                // Get the value and associated weight
                long thisFeature = it.row();

                // If misclassified, hinge loss is active
                if (thisPrediction < 1) {
                    weightDataBuffer[thisFeature] = weightDataBuffer[thisFeature] + it.value() * adjustment;
                }

                // Update weight regardless of classification including scaling based on how common it is
                int thisFeatureCount = featureCountBuffer[thisFeature];
                weightDataBuffer[thisFeature] *= 1 - constScalar / thisFeatureCount;

                // Flag that this segment is dirty
                if (!sgdParams.fullAsync) {
                    size_t offset = it.row() * sizeof(double);
                    memory->flagStateOffsetDirty(WEIGHTS_KEY, nWeightBytes, offset, sizeof(double));
                }
            }
        }

        // Recalculate all predictions
        Map<const RowVectorXd> weights(weightDataBuffer, sgdParams.nWeights);
        MatrixXd prediction = weights * inputs;

        // Persist error
        writeHingeError(memory, sgdParams, batchNumber, outputs, prediction);
    }

    void leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber,
                                  int startIdx, int endIdx) {

        // Always load the inputs and outputs async (as they should be constant)
        Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx, true);
        Map<const MatrixXd> outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, sgdParams.nTrain, startIdx, endIdx, 1,
                                                           true);

        auto weightData = new double[sgdParams.nWeights];
        readMatrixFromState(memory, WEIGHTS_KEY, weightData, 1, sgdParams.nWeights, true);
        Map<MatrixXd> weights(weightData, 1, sgdParams.nWeights);

        // Work out error
        MatrixXd actual = weights * inputs;
        MatrixXd error = actual - outputs;

        // Calculate gradient
        long batchSize = inputs.cols();
        MatrixXd gradient = (2.0 / batchSize) * (error * inputs.transpose());

        // Update weights based on gradient
        for (int w = 0; w < sgdParams.nWeights; w++) {
            double thisGradient = gradient(0, w);

            // Skip if this weight has not contributed
            if (abs(thisGradient) < 0.00000001) {
                continue;
            }

            // Make the update
            weights(0, w) -= sgdParams.learningRate * thisGradient;
            writeMatrixToStateElement(memory, WEIGHTS_KEY, weights, 0, w, true);
        }

        // Make sure all updates have been pushed if we're not running in full async mode
        if (!sgdParams.fullAsync) {
            memory->pushStatePartial(WEIGHTS_KEY);
        }

        // Recalculate the result and return
        MatrixXd prediction = weights * inputs;

        delete[] weightData;

        // Persist error for these examples
        writeSquaredError(memory, sgdParams, batchNumber, outputs, prediction);
    }

    void zeroDoubleArray(FaasmMemory *memory, const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new double[len];
        std::fill(buffer, buffer + len, 0);

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(double), async);
        delete[] buffer;
    }

    void zeroIntArray(FaasmMemory *memory, const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new int[len];
        std::fill(buffer, buffer + len, 0);

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(int), async);
        delete[] buffer;
    }

    void writeFinishedFlag(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber) {
        long totalBytes = sgdParams.nBatches * sizeof(int);
        int finished = 1;
        auto finishedBytes = reinterpret_cast<uint8_t *>(&finished);
        long offset = batchNumber * sizeof(int);

        memory->writeStateOffset(FINISHED_KEY, totalBytes, offset, finishedBytes, sizeof(int), sgdParams.fullAsync);
    }

    void zeroFinished(FaasmMemory *memory, const SgdParams &sgdParams) {
        zeroIntArray(memory, FINISHED_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroErrors(FaasmMemory *memory, const SgdParams &sgdParams) {
        zeroDoubleArray(memory, ERRORS_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroLosses(FaasmMemory *memory, const SgdParams &sgdParams) {
        zeroDoubleArray(memory, LOSSES_KEY, sgdParams.nEpochs, sgdParams.fullAsync);
        zeroDoubleArray(memory, LOSS_TIMESTAMPS_KEY, sgdParams.nEpochs, sgdParams.fullAsync);
    }

    void _writeError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, double error) {
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&error);

        long offset = batchNumber * sizeof(double);
        long totalBytes = sgdParams.nBatches * sizeof(double);

        // Write (going async if in full async mode)
        memory->writeStateOffset(ERRORS_KEY, totalBytes, offset, squaredErrorBytes, sizeof(double),
                                 sgdParams.fullAsync);
    }

    void writeHingeError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction) {
        double err = calculateHingeError(prediction, actual);
        _writeError(memory, sgdParams, batchNumber, err);
    }

    void writeSquaredError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                           const MatrixXd &prediction) {
        double err = calculateSquaredError(prediction, actual);
        _writeError(memory, sgdParams, batchNumber, err);
    }

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load finished flags from state
        const size_t nBytes = sgdParams.nBatches * sizeof(int);
        auto buffer = new uint8_t[nBytes];

        // Read in (async if necessary)
        memory->readState(FINISHED_KEY, buffer, nBytes, sgdParams.fullAsync);

        auto flags = reinterpret_cast<int *>(buffer);

        // Iterate through all the batches to see if finished
        bool allFinished = true;
        auto isFinished = new bool[sgdParams.nBatches];
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double flag = flags[i];

            // If flag is zero, we've not finished
            isFinished[i] = flag > 0;

            if (flag == 0) {
                allFinished = false;
            }
        }

        delete[] isFinished;
        delete[] buffer;
        return allFinished;
    }

    double readTotalError(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load errors from state
        const size_t nBytes = sgdParams.nBatches * sizeof(double);
        auto buffer = new uint8_t[nBytes];

        // Allow fully async
        memory->readState(ERRORS_KEY, buffer, nBytes, sgdParams.fullAsync);

        // Iterate through and sum up
        auto errors = reinterpret_cast<double *>(buffer);
        double totalError = 0;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double error = errors[i];
            totalError += error;
        }

        delete[] buffer;

        return totalError;
    }

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams) {
        double totalSquaredError = readTotalError(memory, sgdParams);

        // Calculate the mean squared error across all batches
        double rmse = std::sqrt(totalSquaredError) / std::sqrt(sgdParams.nTrain);
        return rmse;
    }

    void setUpDummyProblem(FaasmMemory *memory, const SgdParams &params) {
        // Persis the parameters
        writeParamsToState(memory, PARAMS_KEY, params);

        // Create fake training data as dot product of the matrix of training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, params.nWeights);
        Eigen::SparseMatrix<double> inputs = randomSparseMatrix(params.nWeights, params.nTrain, 0.1);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, params.nWeights);

        // Write all data to memory
        writeSparseMatrixToState(memory, INPUTS_KEY, inputs);
        writeMatrixToState(memory, OUTPUTS_KEY, outputs);
        writeMatrixToState(memory, WEIGHTS_KEY, weights);
    }
}

