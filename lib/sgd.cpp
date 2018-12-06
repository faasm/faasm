#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params) {
        size_t nBytes = sizeof(SgdParams);
        auto bytePtr = reinterpret_cast<const uint8_t *>(&params);
        memory->writeState(keyName, bytePtr, nBytes);
    }

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName) {
        size_t nBytes = sizeof(SgdParams);
        auto buffer = new uint8_t[nBytes];

        memory->readState(keyName, buffer, nBytes);

        SgdParams s{};
        memcpy(&s, buffer, nBytes);
        return s;
    }

    MatrixXd hingeLossWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, int epoch, MatrixXd &weights,
                                   const SparseMatrix<double> &inputs, const MatrixXd &outputs) {
        // Create the prediction
        MatrixXd prediction = weights * inputs;

        // Go through each example in the batch
        long batchSize = inputs.cols();
        for (int b = 0; b < batchSize; b++) {
            double thisOutput = outputs(0, b);
            SparseMatrix<double> inputCol = inputs.col(b);

            // Update weights accordingly
            for (int w = 0; w < sgdParams.nWeights; w++) {
                double thisInput = inputCol.coeff(w, 0);

                // Skip if no input here
                if (abs(thisInput) < 0.00000000001) continue;

                double thisWeight = weights.coeff(0, w);

                double thisProduct = thisWeight * thisInput * thisOutput;

                // Do the update. Note that if the product is less than 1, it's a
                // misclassification, so we include this first part of the update
                if (thisProduct < 1) {
                    thisWeight += (sgdParams.learningRate * thisOutput * thisInput);
                }
                thisWeight -= ((sgdParams.learningRate/epoch) * thisWeight);

                // Update in memory and state
                weights(0, w) = thisWeight;
                writeMatrixToStateElement(memory, WEIGHTS_KEY, weights, 0, w);
            }
        }

        // Recalculate the result and return
        MatrixXd postUpdate = weights * inputs;
        return postUpdate;
    }

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, MatrixXd &weights,
                                      const SparseMatrix<double> &inputs, const MatrixXd &outputs) {
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
            writeMatrixToStateElement(memory, WEIGHTS_KEY, weights, 0, w);
        }

        // Recalculate the result and return
        MatrixXd postUpdate = weights * inputs;
        return postUpdate;
    }

    void zeroArray(FaasmMemory *memory, const char *key, long len) {
        // Set buffer to zero
        auto errors = new double[len];
        for (int i = 0; i < len; i++) {
            errors[i] = 0;
        }

        // Write zeroed buffer to state
        auto errorsBytes = reinterpret_cast<uint8_t *>(errors);
        memory->writeState(key, errorsBytes, len * sizeof(double));
        delete[] errors;
    }

    void writeFinishedFlag(FaasmMemory *memory, int workerIdx) {
        double success = 1.0;
        auto successBytes = reinterpret_cast<uint8_t *>(&success);
        long offset = workerIdx * sizeof(double);
        memory->writeStateOffset(FINISHED_KEY, offset, successBytes, sizeof(double));
    }

    void zeroFinished(FaasmMemory *memory, SgdParams sgdParams) {
        zeroArray(memory, FINISHED_KEY, sgdParams.nBatches);
    }

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams) {
        zeroArray(memory, ERRORS_KEY, sgdParams.nBatches);
    }

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams) {
        zeroArray(memory, LOSSES_KEY, sgdParams.nEpochs);
    }

    void _writeError(FaasmMemory *memory, int workerIdx, double error) {
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&error);

        long offset = workerIdx * sizeof(double);
        memory->writeStateOffset(ERRORS_KEY, offset, squaredErrorBytes, sizeof(double));
    }

    void writeHingeError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual) {
        double err = calculateHingeError(actual, outputs);
        _writeError(memory, workerIdx, err);
    }

    void writeSquaredError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual) {
        double err = calculateSquaredError(actual, outputs);
        _writeError(memory, workerIdx, err);
    }

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load finished flags from state
        const size_t nBytes = sgdParams.nBatches * sizeof(double);
        auto buffer = new uint8_t[nBytes];
        memory->readState(FINISHED_KEY, buffer, nBytes);

        auto flags = reinterpret_cast<double *>(buffer);

        // Iterate through
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double flag = flags[i];

            // If error is still zero, we've not yet finished
            if (flag == 0.0) {
                return false;
            }
        }

        return true;
    }

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load errors from state
        const size_t nBytes = sgdParams.nBatches * sizeof(double);
        auto buffer = new uint8_t[nBytes];
        memory->readState(ERRORS_KEY, buffer, nBytes);

        auto errors = reinterpret_cast<double *>(buffer);

        // Iterate through and sum up
        double totalError = 0;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double error = errors[i];
            totalError += error;
        }

        delete[] buffer;

        // Calculate the mean squared error across all batches
        double rmse = sqrt(totalError / sgdParams.nTrain);
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
        writeMatrixToState(memory, OUTPUTS_KEY, outputs);
        writeSparseMatrixToState(memory, INPUTS_KEY, inputs);
        writeMatrixToState(memory, WEIGHTS_KEY, weights);
    }
}

