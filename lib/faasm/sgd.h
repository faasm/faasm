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
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_FULL_ASYNC false


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
        float learningRate;
        float mu = 1.0;
        float learningDecay;
        int nEpochs;
        bool fullAsync;
    };

    SgdParams setUpReutersParams(FaasmMemory *memory, int batchSize, int epochs);

    void writeParamsToState(FaasmMemory *memory, const char *keyName, const SgdParams &params);

    SgdParams readParamsFromState(FaasmMemory *memory, const char *keyName, bool async = false);

    void hingeLossWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            int epoch,
            int batchNumber,
            int startIdx,
            int endIdx
    );

    void leastSquaresWeightUpdate(
            FaasmMemory *memory,
            const SgdParams &sgdParams,
            int batchNumber,
            int startIdx,
            int endIdx
    );

    void zeroErrors(FaasmMemory *memory, const SgdParams &sgdParams);

    void zeroLosses(FaasmMemory *memory, const SgdParams &sgdParams);

    void zeroFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void writeFinishedFlag(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber);

    void writeHingeError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction);

    void writeSquaredError(FaasmMemory *memory, const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                           const MatrixXd &prediction);

    double readTotalError(FaasmMemory *memory, const SgdParams &sgdParams);

    double readRootMeanSquaredError(FaasmMemory *memory, const SgdParams &sgdParams);

    bool readEpochFinished(FaasmMemory *memory, const SgdParams &sgdParams);

    void setUpDummyProblem(FaasmMemory *memory, const SgdParams &params);
}

#endif
