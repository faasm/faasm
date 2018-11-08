#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Initialise params
        SgdParams p;
        p.nWeights = 10;
        p.nTrain = 1000;
        p.nBatches = 50;
        p.maxEpochs = 10;
        p.learningRate = 0.01;

        // Set up dummy data
        setUpDummyProblem(memory, p);

        // Zero the losses
        zeroLosses(memory, p);

        // Begin first epoch
        initCounter(memory, EPOCH_COUNT_KEY);
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}