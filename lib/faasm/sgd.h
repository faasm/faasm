#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/memory.h"
#include "faasm/matrix.h"

using namespace Eigen;

#define EPOCH_COUNT_KEY "epoch_count"
#define WEIGHTS_KEY "weights"
#define INPUTS_KEY "inputs"
#define FEATURE_COUNTS_KEY "feature_counts"
#define OUTPUTS_KEY "outputs"
#define PARAMS_KEY "params"
#define ERRORS_KEY "errors"
#define FINISHED_KEY "finished"
#define LOSSES_KEY "losses"
#define LOSS_TIMESTAMPS_KEY "loss_ts"

// Reuters-specific
//#define REUTERS_N_FEATURES 47236
//#define REUTERS_N_EXAMPLES 781265
#define REUTERS_N_FEATURES 46870
#define REUTERS_N_EXAMPLES 554233
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_FULL_ASYNC true


namespace faasm {
    enum LossType : char {
        RMSE = 1,
        HINGE = 2
    };

    struct SgdParams {
        LossType lossType;
        int nBatches;
        int batchSize;
        int nWeights;
        int nTrain;
        double learningRate;
        double learningDecay;
        int nEpochs;
        bool fullAsync;
    };

    SgdParams setUpReutersParams(FaasmMemory *memory, int batchSize, int epochs);

    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params);

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName, bool async = false);

    MatrixXd hingeLossWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            int epoch,
            const SparseMatrix<double> &inputs,
            const MatrixXd &outputs
    );

    MatrixXd leastSquaresWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            const SparseMatrix<double> &inputs,
            const MatrixXd &outputs
    );

    void zeroErrors(FaasmMemory *memory, const SgdParams &sgdParams);

    void zeroLosses(FaasmMemory *memory, const SgdParams &sgdParams);

    void zeroFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void writeFinishedFlag(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber);

    void writeHingeError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual, const MatrixXd &prediction);

    void writeSquaredError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual, const MatrixXd &prediction);

    double readTotalError(FaasmMemory *memory, const SgdParams &sgdParams);

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams);

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void setUpDummyProblem(FaasmMemory *memory, const SgdParams &params);
}

#endif
