#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/core.h"
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

// Synchronisation barriers
#define BARRIER_COUNT_KEY "barrier_count"
#define MAX_BARRIER_COUNT 60

// Reuters-specific
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_SYNC_INTERVAL 30000


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
        bool fullSync;
        int syncInterval;
    };

    bool getEnvFullAsync();

    bool getEnvFullSync();

    SgdParams setUpReutersParams(int nBatches, int epochs);

    void writeParamsToState(const char *keyName, const SgdParams &params);

    SgdParams readParamsFromState(const char *keyName, bool async);

    void hingeLossWeightUpdate(
            const SgdParams &sgdParams,
            int epoch,
            int batchNumber,
            int startIdx,
            int endIdx
    );

    void leastSquaresWeightUpdate(
            const SgdParams &sgdParams,
            int batchNumber,
            int startIdx,
            int endIdx
    );

    void zeroErrors(const SgdParams &sgdParams);

    void zeroLosses(const SgdParams &sgdParams);

    void zeroFinished(const SgdParams &sgdParams);

    void writeFinishedFlag(const SgdParams &sgdParams, int batchNumber);

    void writeHingeError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction);

    void writeSquaredError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                           const MatrixXd &prediction);

    double readTotalError(const SgdParams &sgdParams);

    double readRootMeanSquaredError(const SgdParams &sgdParams);

    bool readEpochFinished(const SgdParams &sgdParams);

    void setUpDummyProblem(const SgdParams &params);
}

#endif
