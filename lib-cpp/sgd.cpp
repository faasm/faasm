#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
    SgdParams setUpReutersParams(int nBatches, int epochs) {
        // Set up reuters params
        SgdParams p;
        p.nWeights = REUTERS_N_FEATURES;
        p.nTrain = REUTERS_N_EXAMPLES;
        p.learningRate = REUTERS_LEARNING_RATE;
        p.learningDecay = REUTERS_LEARNING_DECAY;
        p.nEpochs = epochs;
        p.mu = 1.0;

        // Round up number of batches
        p.nBatches = nBatches;
        p.batchSize = (REUTERS_N_EXAMPLES + nBatches - 1) / nBatches;

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
        faasmWriteState(keyName, bytePtr, nBytes, true);
    }

    SgdParams readParamsFromState(const char *keyName, bool async) {
        size_t nBytes = sizeof(SgdParams);
        SgdParams s{};
        faasmReadState(keyName, reinterpret_cast<uint8_t *>(&s), nBytes, async);

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
        int updateCount = 0;
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
                // --------- Calculation -----------

                // Get the value and associated weight
                long thisFeature = it.row();

                // If misclassified, hinge loss is active
                if (thisPrediction < 1) {
                    weightDataBuffer[thisFeature] = weightDataBuffer[thisFeature] + it.value() * adjustment;
                }

                // Update weight regardless of classification including scaling based on how common it is
                int thisFeatureCount = featureCountBuffer[thisFeature];
                weightDataBuffer[thisFeature] *= 1 - constScalar / thisFeatureCount;

                // --------- Update -----------

                // Allow completely skipping
                if(sgdParams.syncInterval < 0) {
                    continue;
                }

                // Mark this value as dirty
                size_t offset = it.row() * sizeof(double);
                faasmFlagStateOffsetDirty(WEIGHTS_KEY, nWeightBytes, offset, sizeof(double));

                // Increment the update count and work out if we need to do a sync
                updateCount++;
                bool syncNeeded = (updateCount % sgdParams.syncInterval) != 0;
                if (syncNeeded) {
                    // No sync required
                    continue;
                } else {
                    // Sync the updates
                    faasmPushStatePartial(WEIGHTS_KEY);
                }
            }

            // Final sync
            if (sgdParams.syncInterval >= 0) {
                faasmPushStatePartial(WEIGHTS_KEY);
            }
        }

        // Recalculate all predictions
        Map<const RowVectorXd> weights(weightDataBuffer, sgdParams.nWeights);
        MatrixXd prediction = weights * inputs;

        // Persist error
        writeHingeError(sgdParams, batchNumber, outputs, prediction);
    }

    void zeroDoubleArray(const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new double[len];
        std::fill(buffer, buffer + len, 0);

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        faasmWriteState(key, bytes, len * sizeof(double), async);
    }

    void zeroErrors(const SgdParams &sgdParams) {
        zeroDoubleArray(ERRORS_KEY, sgdParams.nBatches);
    }

    void writeHingeError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction) {
        double err = calculateHingeError(prediction, actual);
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&err);

        long offset = batchNumber * sizeof(double);
        long totalBytes = sgdParams.nBatches * sizeof(double);

        faasmWriteStateOffset(
                ERRORS_KEY,
                totalBytes,
                offset,
                squaredErrorBytes,
                sizeof(double)
        );
    }

    double readTotalError(const SgdParams &sgdParams) {
        // Load errors from state
        auto *errors = new double[sgdParams.nBatches];
        size_t sizeErrors = sgdParams.nBatches * sizeof(double);

        // Allow fully async
        faasmReadState(
                ERRORS_KEY,
                reinterpret_cast<uint8_t *>(errors),
                sizeErrors
        );

        // Iterate through and sum up
        double totalError = 0;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            totalError += errors[i];
        }

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
        writeSparseMatrixToState(INPUTS_KEY, inputs);
        writeMatrixToState(OUTPUTS_KEY, outputs);
        writeMatrixToState(WEIGHTS_KEY, weights);
    }
}

