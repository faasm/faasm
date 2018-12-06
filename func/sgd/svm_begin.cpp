#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Set up reuters params
        SgdParams p;
        p.lossType = HINGE;
        p.nWeights = 21531;
        p.nTrain = 111740;
        p.nBatches = p.nTrain / 2;
        p.nEpochs = 60;
        p.learningRate = 0.1;

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