#include "faasm/sgd.h"

#include <stddef.h>
#include <algorithm>
#include <string.h>

using namespace Eigen;

namespace faasm {
    void writeParamsToState(FaasmMemory *memory, const char *keyName, SgdParams &params) {
        size_t nBytes = sizeof(SgdParams);
        auto bytePtr = reinterpret_cast<uint8_t *>(&params);
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

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, SgdParams &sgdParams, MatrixXd &weights,
                                      const MatrixXd &inputs, const MatrixXd &outputs) {
        // Work out error
        long batchSize = inputs.cols();
        MatrixXd actual = weights * inputs;
        MatrixXd error = actual - outputs;

        // Calculate gradient
        MatrixXd gradient = (-2.0 / batchSize) * (error * inputs.transpose());

        // Update weights based on gradient
        for(int w = 0; w < sgdParams.nWeights; w++) {
            double thisGradient = gradient(0, w);

            // Zero gradient here means none of the inputs in the batch contributed
            if(abs(thisGradient) < 0.00000001) {
                continue;
            }

            // Make the update
            weights(0, w) -= sgdParams.learningRate * thisGradient;
            writeMatrixStateElement(memory, WEIGHTS_KEY, weights, 0, w);
        }

        return actual;
    }

    void setUpDummyProblem(FaasmMemory *memory, SgdParams &params) {
        // Persis the parameters
        writeParamsToState(memory, PARAMS_KEY, params);

        // Create fake training data as dot product of the matrix of training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, params.nWeights);
        Eigen::MatrixXd inputs = randomSparseMatrix(params.nWeights, params.nTrain);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, params.nWeights);

        // Write all data to memory
        writeMatrixState(memory, OUTPUTS_KEY, outputs);
        writeMatrixState(memory, INPUTS_KEY, inputs);
        writeMatrixState(memory, WEIGHTS_KEY, weights);
    }
}

