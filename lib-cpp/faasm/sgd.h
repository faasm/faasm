#ifndef FAASM_SGD_H
#define FAASM_SGD_H

#include "faasm/core.h"
#include "faasm/matrix.h"

using namespace Eigen;

#define WEIGHTS_KEY "weights"
#define INPUTS_KEY "inputs"
#define FEATURE_COUNTS_KEY "feature_counts"
#define OUTPUTS_KEY "outputs"
#define PARAMS_KEY "params"
#define ERRORS_KEY "errors"

// Reuters-specific
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_SYNC_INTERVAL 30000


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

    SgdParams readParamsFromState(const char *keyName);

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
