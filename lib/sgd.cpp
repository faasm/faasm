#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
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

    MatrixXd hingeLossWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, int epoch,
                                   const SparseMatrix<double> &inputs, const MatrixXd &outputs) {

        auto weightDataBuffer = new double[sgdParams.nWeights];
        auto weightDataByteBuffer = reinterpret_cast<uint8_t *>(weightDataBuffer);
        size_t nWeightBytes = sgdParams.nWeights * sizeof(double);

        // Read in the weights initially
        memory->readState(WEIGHTS_KEY, weightDataByteBuffer, nWeightBytes, true);
        Map<RowVectorXd> weights(weightDataBuffer, sgdParams.nWeights);

        // Iterate through all training examples (i.e. columns)
        for (int col = 0; col < inputs.outerSize(); ++col) {
            // Read in weights asynchronously *directly into the existing weights matrix*
            memory->readState(WEIGHTS_KEY, weightDataByteBuffer, nWeightBytes, true);

            // Get input and output associated with this example
            double thisOutput = outputs.coeff(0, col);
            SparseVector<double> thisInput = inputs.col(col);

            // Work out the prediction for this example. This will be a single number.
            // Do this inside the loop to include weight updates. Importantly the input
            // here will be sparse so we need to use the eigen multiplication
            Matrix<double, 1, 1> prediction = weights * thisInput;
            double thisPrediction = prediction.coeff(0, 0);

            // If the prediction multiplied by the output is less than one, it's misclassified
            bool isMisclassified = (thisOutput * thisPrediction) < 1;

            // Iterate through all non-zero input values in this column
            for (Eigen::SparseMatrix<double>::InnerIterator it(inputs, col); it; ++it) {
                // Get the value and associated weight
                double thisValue = it.value();
                double thisWeight = weightDataBuffer[it.row()];

                // If misclassified, hinge loss is active
                if (isMisclassified) {
                    thisWeight += (sgdParams.learningRate * thisOutput * thisValue);
                }

                thisWeight *= (1 - (sgdParams.learningRate / (1 + epoch)));

                // Write update memory array
                weightDataBuffer[it.row()] = thisWeight;

                // Update in state
                auto byteWeight = reinterpret_cast<uint8_t *>(&thisWeight);
                size_t offset = it.row() * sizeof(double);
                memory->writeStateOffset(WEIGHTS_KEY, nWeightBytes, offset, byteWeight, sizeof(double), true);
            }
        }

        // Make sure all updates have been pushed if we're not running in full async mode
        if (!sgdParams.fullAsync) {
            memory->pushStatePartial(WEIGHTS_KEY);
        }

        // Recalculate the result and return
        MatrixXd postUpdate = weights * inputs;

        delete[] weightDataBuffer;

        return postUpdate;
    }

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams,
                                      const SparseMatrix<double> &inputs, const MatrixXd &outputs) {

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
        MatrixXd postUpdate = weights * inputs;

        delete[] weightData;

        return postUpdate;
    }

    void zeroDoubleArray(FaasmMemory *memory, const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new double[len];
        for (int i = 0; i < len; i++) {
            buffer[i] = 0;
        }

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(double), async);
        delete[] buffer;
    }

    void zeroIntArray(FaasmMemory *memory, const char *key, long len, bool async) {
        // Set buffer to zero
        auto buffer = new int[len];
        for (int i = 0; i < len; i++) {
            buffer[i] = 0;
        }

        // Write zeroed buffer to state
        auto bytes = reinterpret_cast<uint8_t *>(buffer);
        memory->writeState(key, bytes, len * sizeof(int), async);
        delete[] buffer;
    }

    void writeFinishedFlag(FaasmMemory *memory, SgdParams sgdParams, int batchNumber) {
        long totalBytes = sgdParams.nBatches * sizeof(int);
        int finished = 1;
        auto finishedBytes = reinterpret_cast<uint8_t *>(&finished);
        long offset = batchNumber * sizeof(int);

        // Async if necessary
        memory->writeStateOffset(FINISHED_KEY, totalBytes, offset, finishedBytes, sizeof(int), sgdParams.fullAsync);
    }

    void zeroFinished(FaasmMemory *memory, SgdParams sgdParams) {
        zeroIntArray(memory, FINISHED_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams) {
        zeroDoubleArray(memory, ERRORS_KEY, sgdParams.nBatches, sgdParams.fullAsync);
    }

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams) {
        zeroDoubleArray(memory, LOSSES_KEY, sgdParams.nEpochs, sgdParams.fullAsync);
    }

    void _writeError(FaasmMemory *memory, SgdParams sgdParams, int batchNumber, double error) {
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&error);

        long offset = batchNumber * sizeof(double);
        long totalBytes = sgdParams.nBatches * sizeof(double);

        // Write (going async if in full async mode)
        memory->writeStateOffset(ERRORS_KEY, totalBytes, offset, squaredErrorBytes, sizeof(double),
                                 sgdParams.fullAsync);
    }

    void writeHingeError(FaasmMemory *memory, SgdParams sgdParams, int batchNumber, const MatrixXd &outputs,
                         const MatrixXd &actual) {
        double err = calculateHingeError(actual, outputs);
        _writeError(memory, sgdParams, batchNumber, err);
    }

    void writeSquaredError(FaasmMemory *memory, SgdParams sgdParams, int batchNumber, const MatrixXd &outputs,
                           const MatrixXd &actual) {
        double err = calculateSquaredError(actual, outputs);
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

        // Allow fully async
        memory->readState(ERRORS_KEY, buffer, nBytes, sgdParams.fullAsync);

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

