#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/memory.h"
#include "faasm/matrix.h"

using namespace Eigen;

#define EPOCH_COUNT_KEY "epoch_count"
#define WEIGHTS_KEY "weights"
#define INPUTS_KEY "inputs"
#define OUTPUTS_KEY "outputs"
#define PARAMS_KEY "params"
#define ERRORS_KEY "errors"
#define FINISHED_KEY "finished"
#define LOSSES_KEY "losses"
#define LOSS_TIMESTAMPS_KEY "loss_ts"

// Reuters-specific
#define REUTERS_N_FEATURES 21531
#define REUTERS_N_EXAMPLES 111740
#define REUTERS_LEARNING_RATE 0.1


namespace faasm {
    enum LossType : char {
        RMSE = 1,
        HINGE = 2
    };

    struct SgdParams {
        LossType lossType;
        int nBatches;
        int nWeights;
        int nTrain;
        double learningRate;
        int nEpochs;
    };

    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params);

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName);

    MatrixXd hingeLossWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            int epoch,
            MatrixXd &weights,
            const SparseMatrix<double> &inputs,
            const MatrixXd &outputs
    );

    MatrixXd leastSquaresWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            MatrixXd &weights,
            const SparseMatrix<double> &inputs,
            const MatrixXd &outputs
    );

    void zeroErrors(FaasmMemory *memory, SgdParams sgdParams);

    void zeroLosses(FaasmMemory *memory, SgdParams sgdParams);

    void zeroFinished(FaasmMemory *memory, SgdParams sgdParams);

    void writeFinishedFlag(FaasmMemory *memory, int workerIdx);

    void writeHingeError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual);

    void writeSquaredError(FaasmMemory *memory, int workerIdx, const MatrixXd &outputs, const MatrixXd &actual);

    double readTotalError(FaasmMemory *memory, const SgdParams &sgdParams);

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams);

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void setUpDummyProblem(FaasmMemory *memory, const SgdParams &params);
}

#endif
