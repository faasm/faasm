#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/core.h"
#include "faasm/matrix.h"

using namespace Eigen;

#define WEIGHTS_KEY "weights"
#define MASK_KEY "weights_mask"
#define INPUTS_KEY "inputs"
#define FEATURE_COUNTS_KEY "feature_counts"
#define OUTPUTS_KEY "outputs"
#define PARAMS_KEY "params"
#define ERRORS_KEY "errors"

// Reuters-specific
// Note that the sync interval determines how often workers will
// sync with the remote storage. There are about 60 million updates
// to be performed in each epoch, and a possible 47k features on which
// these updates can occur. With lots of colocated workers these syncs
// can be relatively infrequent, but with lots of distributed workers the
// syncs need to be more frequent.
//
// Sync interval of -1 means no syncing
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265

// #define REUTERS_SYNC_INTERVAL -1
#define REUTERS_SYNC_INTERVAL 60000000


namespace faasm {
    struct SgdParams {
        int nBatches;
        int batchSize;
        int nWeights;
        int nTrain;
        float learningRate;
        float mu = 1.0;
        float learningDecay;
        int nEpochs;
        int syncInterval;
    };

    SgdParams setUpReutersParams(int nBatches, int epochs, bool push);

    void writeParamsToState(const char *keyName, const SgdParams &params, bool push);

    SgdParams readParamsFromState(const char *keyName, bool pull);

    void hingeLossWeightUpdate(
            const SgdParams &sgdParams,
            int epoch,
            int batchNumber,
            int startIdx,
            int endIdx
    );

    void zeroErrors(const SgdParams &sgdParams, bool push);

    void writeHingeError(const SgdParams &sgdParams, int batchNumber, const MatrixXd &actual,
                         const MatrixXd &prediction);

    double readTotalError(const SgdParams &sgdParams);

    double readRootMeanSquaredError(const SgdParams &sgdParams);

    void setUpDummyProblem(const SgdParams &params);
}

#endif
