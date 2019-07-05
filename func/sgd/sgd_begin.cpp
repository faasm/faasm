#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

FAASM_MAIN_FUNC() {
    // Initialise params
    faasm::SgdParams p;
    p.lossType = faasm::RMSE;
    p.nWeights = 10;
    p.nTrain = 10000;
    p.batchSize = 20;
    p.nBatches = 500;
    p.nEpochs = 20;
    p.learningRate = 0.01;

    // Set up dummy data
    setUpDummyProblem(p);

    // Zero the losses
    zeroLosses(p);

    // Begin first epoch
    faasm::initCounter(EPOCH_COUNT_KEY, p.fullAsync);
    faasmChainFunction("sgd_epoch");

    return 0;
}
