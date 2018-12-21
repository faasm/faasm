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

        delete[] buffer;
        return s;
    }

    MatrixXd hingeLossWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, int epoch,
                                   const SparseMatrix<double> &inputs, const MatrixXd &outputs) {
        // Iterate through all training examples (i.e. columns)
        for (int col = 0; col < inputs.outerSize(); ++col) {
            // Read in weights asynchronously
            MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, sgdParams.nWeights, true);

            // Get input and output associated with this example
            double thisOutput = outputs.coeff(0, col);
            SparseMatrix<double> thisInput = inputs.col(col);

            // Work out the prediction for this example (do this inside the loop to include weight updates
            MatrixXd prediction = weights * thisInput;
            double thisPrediction = prediction.coeff(0, 0);

            // If the prediction multiplied by the output is less than one, it's misclassified
            bool isMisclassified = (thisOutput * thisPrediction) < 1;

            // Iterate through all non-zero input values in this column
            for (Eigen::SparseMatrix<double>::InnerIterator it(inputs, col); it; ++it) {
                // Get the value and associated weight
                double thisValue = it.value();
                double thisWeight = weights.coeff(0, it.row());

                // If misclassified, hinge loss is active
                if (isMisclassified) {
                    thisWeight += (sgdParams.learningRate * thisOutput * thisValue);
                }

                thisWeight *= (1 - (sgdParams.learningRate / (1 + epoch)));

                // Update in memory
                weights(0, it.row()) = thisWeight;

                writeMatrixToStateElement(memory, WEIGHTS_KEY, weights, 0, it.row(), true);
            }
        }

        // Recalculate the result and return
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, sgdParams.nWeights, true);
        MatrixXd postUpdate = weights * inputs;
        return postUpdate;
    }

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams,
                                      const SparseMatrix<double> &inputs, const MatrixXd &outputs) {
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, sgdParams.nWeights, true);

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

    void zeroDoubleArray(FaasmMemory *memory, const char *key, long len) {
        // Set buffer to zero
        auto buffer = new double[len];
        for (int i = 0; i < len; i++) {
            buffer[i] = 0;
        }

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(double));
        delete[] buffer;
    }

    void zeroIntArray(FaasmMemory *memory, const char *key, long len) {
        // Set buffer to zero
        auto buffer = new int[len];
        for (int i = 0; i < len; i++) {
            buffer[i] = 0;
        }

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(int));
        delete[] buffer;
    }

    void writeFinishedFlag(FaasmMemory *memory, int batchNumber) {
        int finished = 1;
        auto finishedBytes = reinterpret_cast<uint8_t *>(&finished);
        long offset = batchNumber * sizeof(int);
        memory->writeStateOffset(FINISHED_KEY, offset, finishedBytes, sizeof(int));
    }

    void zeroFinished(FaasmMemory *memory, SgdParams sgdParams) {
        zeroIntArray(memory, FINISHED_KEY, sgdParams.nBatches);
    }

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams) {
        zeroDoubleArray(memory, ERRORS_KEY, sgdParams.nBatches);
    }

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams) {
        zeroDoubleArray(memory, LOSSES_KEY, sgdParams.nEpochs);
    }

    void _writeError(FaasmMemory *memory, int batchNumber, double error) {
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&error);

        long offset = batchNumber * sizeof(double);
        memory->writeStateOffset(ERRORS_KEY, offset, squaredErrorBytes, sizeof(double));
    }

    void writeHingeError(FaasmMemory *memory, int batchNumber, const MatrixXd &outputs, const MatrixXd &actual) {
        double err = calculateHingeError(actual, outputs);
        _writeError(memory, batchNumber, err);
    }

    void writeSquaredError(FaasmMemory *memory, int batchNumber, const MatrixXd &outputs, const MatrixXd &actual) {
        double err = calculateSquaredError(actual, outputs);
        _writeError(memory, batchNumber, err);
    }

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load finished flags from state
        const size_t nBytes = sgdParams.nBatches * sizeof(int);
        auto buffer = new uint8_t[nBytes];
        memory->readState(FINISHED_KEY, buffer, nBytes);

        auto flags = reinterpret_cast<int *>(buffer);

        // Iterate through all the batches to see if finished
        bool isFinished = true;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double flag = flags[i];

            // If flag is zero, we've not finished
            if (flag == 0) {
                isFinished = false;
                break;
            }
        }

        delete[] buffer;
        return isFinished;
    }

    double readTotalError(FaasmMemory *memory, const SgdParams &sgdParams) {
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

        return totalError;
    }

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams) {
        double totalError = readTotalError(memory, sgdParams);

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
        writeSparseMatrixToState(memory, INPUTS_KEY, inputs);
        writeMatrixToState(memory, OUTPUTS_KEY, outputs);
        writeMatrixToState(memory, WEIGHTS_KEY, weights);
    }
}

