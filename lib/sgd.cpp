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

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, MatrixXd &weights,
                                      const SparseMatrix<double> &inputs, const MatrixXd &outputs) {
        // Work out error
        long batchSize = inputs.cols();
        MatrixXd actual = weights * inputs;
        MatrixXd error = actual - outputs;

        // Calculate gradient
        MatrixXd gradient = (2.0 / batchSize) * (error * inputs.transpose());

        // Update weights based on gradient
        for(int w = 0; w < sgdParams.nWeights; w++) {
            double thisGradient = gradient(0, w);

            // Zero gradient here means none of the inputs in the batch contributed
            if(abs(thisGradient) < 0.00000001) {
                continue;
            }

            // Make the update
            weights(0, w) -= sgdParams.learningRate * thisGradient;
            writeMatrixToStateElement(memory, WEIGHTS_KEY, weights, 0, w);
        }

        return actual;
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

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams) {
        zeroArray(memory, ERRORS_KEY, sgdParams.nBatches);
    }

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams) {
        zeroArray(memory, LOSSES_KEY, sgdParams.nBatches);
    }

    void writeSquaredError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual) {
        double squaredError = calculateSquaredError(actual, outputs);
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&squaredError);
        
        long offset = workerIdx * sizeof(double);
        memory->writeStateOffset(ERRORS_KEY, offset, squaredErrorBytes, sizeof(double));
    }

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams) {
        // Load errors from state
        const size_t nBytes = sgdParams.nBatches * sizeof(double);
        auto buffer = new uint8_t[nBytes];
        memory->readState(ERRORS_KEY, buffer, nBytes);

        auto errors = reinterpret_cast<double*>(buffer);

        // Iterate through and sum up
        double totalError = 0;
        for (int i = 0; i < sgdParams.nBatches; i++) {
            double error = errors[i];

            // If error is still zero, we've not yet finished
            if (error == 0.0) {
                return 0;
            }

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

