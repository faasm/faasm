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
#define ERRORS_KEY "errors"
#define FINISHED_KEY "finished"
#define LOSSES_KEY "losses"


namespace faasm {
    struct SgdParams {
        int nBatches = 10; // Number of batches in each epoch
        int nWeights = 10; // Number of weights
        int nTrain = 1000; // Number of training examples
        double learningRate = 0.1;
        int nEpochs = 10;
    };

    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params);

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName);

    MatrixXd leastSquaresWeightUpdate(FaasmMemory *memory, const SgdParams &sgdParams, MatrixXd &weights,
                                        const SparseMatrix<double> &inputs,
                                        const MatrixXd &outputs);

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams);

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams);

    void zeroFinished(FaasmMemory *memory, SgdParams sgdParams);

    void writeFinishedFlag(FaasmMemory *memory, int workerIdx);

    void writeSquaredError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual);

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams);

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void setUpDummyProblem(FaasmMemory *memory, const SgdParams &params);
}

#endif
