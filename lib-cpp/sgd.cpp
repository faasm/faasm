#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
    bool _getEnvVarBool(const char *varName) {
        char envVal[3];
        faasmReadConfig(varName, envVal);

        return strcmp(envVal, "1") == 0;
    }

    bool getEnvFullSync() {
        bool result = _getEnvVarBool("FULL_SYNC");

        if (result) {
            printf("SGD running full sync\n");
        } else {
            printf("SGD not running full sync\n");
        }

        return result;
    }

    bool getEnvFullAsync() {
        bool result = _getEnvVarBool("FULL_ASYNC");

        if (result) {
            printf("SGD running full async\n");
        } else {
            printf("SGD not running full async\n");
        }

        return result;
    }

    SgdParams setUpReutersParams(int nBatches, int epochs) {
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

        // Full async or not
        p.fullAsync = getEnvFullAsync();
        p.fullSync = getEnvFullSync();

        // How many examples should be processed before doing a synchronous read
        // to update worker's weights
        p.syncInterval = REUTERS_SYNC_INTERVAL;

        // Write params (will take async/ not from params themselves)
        printf("Writing SVM params to state\n");
        writeParamsToState(PARAMS_KEY, p);

        return p;
    }

    void writeParamsToState(const char *keyName, const SgdParams &params) {
        size_t nBytes = sizeof(SgdParams);
        auto bytePtr = reinterpret_cast<const uint8_t *>(&params);

        // Allow full async
        faasmWriteState(keyName, bytePtr, nBytes, params.fullAsync);
    }

    SgdParams readParamsFromState(const char *keyName, bool async) {
        size_t nBytes = sizeof(SgdParams);
        auto buffer = new uint8_t[nBytes];

        faasmReadState(keyName, buffer, nBytes, async);

        SgdParams s{};
        memcpy(&s, buffer, nBytes);

        delete[] buffer;
        return s;
    }

    void hingeLossWeightUpdate(const SgdParams &sgdParams, int epoch, int batchNumber,
                               int startIdx, int endIdx) {

        // Load this batch of inputs
        printf("Loading inputs %i - %i\n", startIdx, endIdx);
        Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(INPUTS_KEY,
                                                                                  startIdx, endIdx, true);

        // Load this batch of outputs
        printf("Loading outputs %i - %i\n", startIdx, endIdx);
        Map<const MatrixXd> outputs = readMatrixColumnsFromState(OUTPUTS_KEY, sgdParams.nTrain,
                                                                 startIdx, endIdx, 1, true);

        // Load the weights
        size_t nWeightBytes = sgdParams.nWeights * sizeof(double);
        uint8_t *weightDataByteBuffer = faasmReadStatePtr(WEIGHTS_KEY, nWeightBytes, true);

        auto weightDataBuffer = reinterpret_cast<double *>(weightDataByteBuffer);

        // Read in the feature counts (will be constant)
        size_t nFeatureCountBytes = sgdParams.nWeights * sizeof(int);
        uint8_t *featureCountByteBuffer = faasmReadStatePtr(FEATURE_COUNTS_KEY, nFeatureCountBytes, true);
        auto featureCountBuffer = reinterpret_cast<int *>(featureCountByteBuffer);

        // Shuffle examples in this batch
        int *cols = randomIntRange(inputs.outerSize());

        // Iterate through all training examples (i.e. columns)
        int exampleCount = 0;
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

                // Handle updates differently for full sync/ async/ full async
                if (sgdParams.fullSync) {
                    // Write offset synchronously on every loop if running full sync
                    size_t offset = it.row() * sizeof(double);
                    double *thisWeightPtr = &weightDataBuffer[thisFeature];
                    auto weightBytes = reinterpret_cast<uint8_t *>(thisWeightPtr);

                    faasmWriteStateOffset(WEIGHTS_KEY, nWeightBytes, offset, weightBytes, sizeof(double), false);
                } else if (!sgdParams.fullAsync) {
                    // Flag that this segment is dirty if async
                    size_t offset = it.row() * sizeof(double);
                    faasmFlagStateOffsetDirty(WEIGHTS_KEY, nWeightBytes, offset, sizeof(double));
                }
            }

            // If we're running in normal async mode, do a write (of the dirty segments)
            // This will both push local writes, and pull in the latest updates
            if (!sgdParams.fullSync && !sgdParams.fullAsync) {
                exampleCount++;
                bool isSyncNeeded = (exampleCount > 0) && ((exampleCount % sgdParams.syncInterval) == 0);
                if (isSyncNeeded) {
                    faasmPushStatePartial(WEIGHTS_KEY);
                }
            }
        }

        // Recalculate all predictions
        printf("Calculating error\n");
        Map<const RowVectorXd> weights(weightDataBuffer, sgdParams.nWeights);
        MatrixXd prediction = weights * inputs;

        // Persist error
        writeHingeError(sgdParams, batchNumber, outputs, prediction);
    }

    void leastSquaresWeightUpdate(const SgdParams &sgdParams, int batchNumber,
                                  int startIdx, int endIdx) {

        // Always load the inputs and outputs async (as they should be constant)
        Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(INPUTS_KEY, startIdx, endIdx,
                                                                                  true);
        Map<const MatrixXd> outputs = readMatrixColumnsFromState(OUTPUTS_KEY, sgdParams.nTrain, startIdx,
                                                                 endIdx, 1,
                                                                 true);

        auto weightData = new double[sgdParams.nWeights];
        readMatrixFromState(WEIGHTS_KEY, weightData, 1, sgdParams.nWeights, true);
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
            writeMatrixToStateElement(WEIGHTS_KEY, weights, 0, w, true);
        }

        // Make sure all updates have been pushed if we're not running in full async mode
        if (!sgdParams.fullAsync) {
            faasmPushStatePartial(WEIGHTS_KEY);
        }

        // Recalculate the result and return
        MatrixXd prediction = weights * inputs;

        delete[] weightData;

        // Persist error for these examples
        writeSquaredError(sgdParams, batchNumber, outputs, prediction);
    }

    void zeroDoubleArray(const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new double[len];
        std::fill(buffer, buffer + len, 0);

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        faasmWriteState(key, bytes, len * sizeof(double), async);

        delete[] buffer;
    }

    void zeroIntArray(const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new int[len];
        std::fill(buffer, buffer + len, 0);

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        faasmWriteState(key, bytes, len * sizeof(int), async);
        delete[] buffer;
    }

    void writeFinishedFlag(const SgdParams &sgdParams, int batchNumber) {
        long totalBytes = sgdParams.nBatches * sizeof(int);
        int finished = 1;
        auto finishedBytes = reinterpret_cast<uint8_t *>(&finished);
        long offset = batchNumber * sizeof(int);

        faasmWriteStateOffset(FINISHED_KEY, totalBytes, offset, finishedBytes, sizeof(int), sgdParams.fullAsync);
    }

    void zeroFinished(const SgdParams &sgdParams) {
        zeroIntArray(FINISHED_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroErrors(const SgdParams &sgdParams) {
        zeroDoubleArray(ERRORS_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroLosses(const SgdParams &sgdParams) {
        zeroDoubleArray(LOSSES_KEY, sgdParams.nEpochs, sgdParams.fullAsync);
        zeroDoubleArray(LOSS_TIMESTAMPS_KEY, sgdParams.nEpochs, sgdParams.fullAsync);
    }

    void _writeError(const SgdParams &sgdParams, int batchNumber, double error) {
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&error);

        long offset = batchNumber * sizeof(double);
        long totalBytes = sgdParams.nBatches * sizeof(double);

        // Write (going async if in full async mode)
        faasmWriteStateOffset(ERRORS_KEY, totalBytes, offset, squaredErrorBytes, sizeof(double),
                                 sgdParams.fullAsync);
    }

    void writeHingeError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction) {
        double err = calculateHingeError(prediction, actual);
        _writeError(sgdParams, batchNumber, err);
    }

    void writeSquaredError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                           const MatrixXd &prediction) {
        double err = calculateSquaredError(prediction, actual);
        _writeError(sgdParams, batchNumber, err);
    }

    bool readEpochFinished(const SgdParams &sgdParams) {
        // Load finished flags from state
        const size_t nBytes = sgdParams.nBatches * sizeof(int);
        auto buffer = new uint8_t[nBytes];

        // Read in
        faasmReadState(FINISHED_KEY, buffer, nBytes, sgdParams.fullAsync);

        auto flags = reinterpret_cast<int *>(buffer);

        // Iterate through all the batches to see if finished
        bool allFinished = true;
        auto isFinished = new bool[sgdParams.nBatches];
        for (int i = 0; i < sgdParams.nBatches; i++) {
            int flag = flags[i];

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

    double readTotalError(const SgdParams &sgdParams) {
        // Load errors from state
        const size_t nBytes = sgdParams.nBatches * sizeof(double);
        auto buffer = new uint8_t[nBytes];

        // Allow fully async
        faasmReadState(ERRORS_KEY, buffer, nBytes, sgdParams.fullAsync);

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

    double readRootMeanSquaredError(const SgdParams &sgdParams) {
        double totalSquaredError = readTotalError(sgdParams);

        // Calculate the mean squared error across all batches
        double rmse = std::sqrt(totalSquaredError) / std::sqrt(sgdParams.nTrain);
        return rmse;
    }

    void setUpDummyProblem(const SgdParams &params) {
        // Persis the parameters
        writeParamsToState(PARAMS_KEY, params);

        // Create fake training data as dot product of the matrix of training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, params.nWeights);
        Eigen::SparseMatrix<double> inputs = randomSparseMatrix(params.nWeights, params.nTrain, 0.1);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, params.nWeights);

        // Write all data to memory
        writeSparseMatrixToState(INPUTS_KEY, inputs, params.fullAsync);
        writeMatrixToState(OUTPUTS_KEY, outputs, params.fullAsync);
        writeMatrixToState(WEIGHTS_KEY, weights, params.fullAsync);
    }
}

