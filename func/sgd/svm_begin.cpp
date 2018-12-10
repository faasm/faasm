#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Set up reuters params
        SgdParams p;
        p.lossType = HINGE;
        p.nWeights = REUTERS_N_FEATURES;
        p.nTrain = REUTERS_N_EXAMPLES;
        p.learningRate = REUTERS_LEARNING_RATE;

        // Largest factors of number of training examples: 1, 2, 4, 5, 10, 20, 37, 74, 148, 151, 185, 302, 370, 604,
        // 740, 755, 1510, 3020, 5587, 11174, 22348, 27935, 55870, 111740
        p.nBatches = p.nTrain / 604;
        p.nEpochs = 60;

        writeParamsToState(memory, PARAMS_KEY, p);

        // Initialise weights
        Eigen::MatrixXd weights = randomDenseMatrix(1, p.nWeights);
        writeMatrixToState(memory, WEIGHTS_KEY, weights);

        // Zero the losses
        zeroLosses(memory, p);

        // Begin first epoch
        initCounter(memory, EPOCH_COUNT_KEY);
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}