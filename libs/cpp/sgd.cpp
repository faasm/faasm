#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>
#include <faasm/state.h>

using namespace Eigen;

namespace faasm {
    void writeParamsToState(const char *keyName, const SgdParams &params, bool push) {
        size_t nBytes = sizeof(SgdParams);
        auto bytePtr = reinterpret_cast<const uint8_t *>(&params);

        faasmWriteState(keyName, bytePtr, nBytes);

        if (push) {
            faasmPushState(keyName);
        }
    }

    SgdParams readParamsFromState(const char *keyName, bool pull) {
        size_t nBytes = sizeof(SgdParams);

        if (pull) {
            faasmPullState(keyName, nBytes);
        }

        SgdParams s{};
        faasmReadState(keyName, BYTES(&s), nBytes);

        return s;
    }

    void hingeLossWeightUpdate(const SgdParams &sgdParams, int startIdx, int endIdx) {
        /* Note this is always asynchronous with pushes decided based on the params */
        
        // Load this batch of inputs (read-only)
        printf("Loading inputs %i - %i\n", startIdx, endIdx);
        Map<const SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(INPUTS_KEY, startIdx, endIdx, false);

        // Load this batch of outputs (read-only)
        printf("Loading outputs %i - %i\n", startIdx, endIdx);
        Map<const MatrixXd> outputs = readMatrixColumnsFromState(OUTPUTS_KEY, sgdParams.nTrain, startIdx, endIdx, 1,
                                                                 false);

        // Read in the feature counts (will be constant)
        printf("Loading feature counts\n");
        size_t nFeatureCountBytes = sgdParams.nWeights * sizeof(int);
        uint8_t *featureCountByteBuffer = faasmReadStatePtr(FEATURE_COUNTS_KEY, nFeatureCountBytes);
        auto featureCountBuffer = reinterpret_cast<int *>(featureCountByteBuffer);

        // Get pointers to the weights and mask (note that the mask state will only ever exist locally
        // and is create if not already present).
        printf("Loading weights\n");
        size_t nWeightBytes = sgdParams.nWeights * sizeof(double);
        uint8_t *weightDataByteBuffer = faasmReadStatePtr(WEIGHTS_KEY, nWeightBytes);
        auto weightDataBuffer = reinterpret_cast<double *>(weightDataByteBuffer);

        // uint8_t *weightMaskBytes = faasmReadStatePtr(MASK_KEY, nWeightBytes);
        // auto weightMask = reinterpret_cast<unsigned int *>(weightMaskBytes);

        // TODO - async conflict
        // Zero the mask
        // memset(weightMaskBytes, 0, nWeightBytes);

        // Shuffle examples in this batch
        printf("Shuffling cols\n");
        int *cols = randomIntRange(inputs.outerSize());

        // Iterate through all training examples (i.e. columns)
        printf("Iterating on SGD\n");
        int updateCount = 0;
        for (int c = 0; c < inputs.outerSize(); ++c) {
            int col = cols[c];

            double thisOutput = outputs.coeff(0, col);
            // Iterate through the non-null rows in this column
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

                // Ignore if we're not syncing
                if (sgdParams.syncInterval < 0) {
                    continue;
                }

                // Flag this chunk as dirty
                faasmFlagStateOffsetDirty(WEIGHTS_KEY, nWeightBytes, thisFeature * sizeof(double), sizeof(double));
                // faasm::maskDouble(weightMask, thisFeature);

                // Increment the update count and work out if we need to do a sync
                updateCount++;
                bool syncNeeded = (updateCount > 0) && (updateCount % sgdParams.syncInterval) == 0;
                if (syncNeeded) {
                    // TODO - Async conflict
                    faasmPushStatePartial(WEIGHTS_KEY);

                    // Sync the updates
                    // faasmFlagStateDirty(WEIGHTS_KEY, nWeightBytes);
                    // faasmPushStatePartialMask(WEIGHTS_KEY, MASK_KEY);
                } else {
                    // No sync required
                    continue;
                }
            }
        }

        // TODO - Async conflict
        // Final sync if we're doing syncs
        if (sgdParams.syncInterval >= 0) {
            faasmPushStatePartial(WEIGHTS_KEY);
            // faasmFlagStateDirty(WEIGHTS_KEY, nWeightBytes);
            // faasmPushStatePartialMask(WEIGHTS_KEY, MASK_KEY);
        }

        // Recalculate all predictions
        Map<const RowVectorXd> weights(weightDataBuffer, sgdParams.nWeights);
        MatrixXd prediction = weights * inputs;

        // Write error
        writeHingeError(sgdParams, outputs, prediction);
    }

    void writeHingeError(const SgdParams &sgdParams, const MatrixXd &actual, const MatrixXd &prediction) {
        double err = calculateHingeError(prediction, actual);
        auto squaredErrorBytes = BYTES(&err);

        faasmAppendState(
                ERRORS_KEY,
                squaredErrorBytes,
                sizeof(double)
        );
    }

    double readRootMeanSquaredError(const SgdParams &sgdParams) {
        // Load errors from state
        auto *errors = new double[sgdParams.nBatches];
        size_t sizeErrors = sgdParams.nBatches * sizeof(double);

        // Make sure filled with zeros
        memset(errors, 0, sizeErrors);

        faasmReadAppendedState(
                ERRORS_KEY,
                BYTES(errors),
                sizeErrors,
                sgdParams.nBatches
        );

        // Iterate through and sum up
        double totalError = 0.0;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            totalError += errors[i];
        }

        // Calculate the mean squared error across all batches
        double rmse = std::sqrt(totalError) / std::sqrt(sgdParams.nTrain);
        return rmse;
    }

    void setUpDummyProblem(const SgdParams &params) {
        // Persist the parameters
        writeParamsToState(PARAMS_KEY, params, true);

        // Create fake training data as dot product of the matrix of training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, params.nWeights);
        Eigen::SparseMatrix<double> inputs = randomSparseMatrix(params.nWeights, params.nTrain, 0.5);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, params.nWeights);

        // Write all data to memory
        writeSparseMatrixToState(INPUTS_KEY, inputs, true);
        writeMatrixToState(OUTPUTS_KEY, outputs, true);
        writeMatrixToState(WEIGHTS_KEY, weights, true);

        // Set up fake feature counts
        auto featureCounts = new int[params.nWeights];
        for(int i = 0; i < params.nWeights; i++) {
            featureCounts[i] = randomInteger(1, 200);
        }

        // Write feature counts to state
        auto featureBytes = BYTES(featureCounts);
        faasmWriteState(FEATURE_COUNTS_KEY, featureBytes, params.nWeights * sizeof(int));
        delete[] featureCounts;
    }
}

