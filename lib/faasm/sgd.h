#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/memory.h"
#include "faasm/matrix.h"

using namespace Eigen;

#define EPOCH_COUNT_KEY "epochCount"
#define WEIGHTS_KEY "weights"
#define INPUTS_KEY "inputs"
#define OUTPUTS_KEY "outputs"
#define PARAMS_KEY "params"

namespace faasm {
    struct SgdParams {
        int nBatches = 10; // Number of batches in each epoch
        int nWeights = 10; // Number of weights
        int nTrain = 1000; // Number of training examples
        double learningRate = 0.1;
        int maxEpochs = 10;
    };

    void writeParamsToState(FaasmMemory *memory, const char *keyName, SgdParams &params);

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName);

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, SgdParams &sgdParams, MatrixXd &weights,
                                        const MatrixXd &inputs,
                                        const MatrixXd &outputs);

    void setUpDummyProblem(FaasmMemory *memory, SgdParams &params);
}

#endif
