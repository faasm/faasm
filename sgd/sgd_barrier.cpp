#include "faasm.h"
#include "matrix.h"
#include "counter.h"
#include "sgd_constants.h"


namespace faasm {
    bool checkWorkers(FaasmMemory *memory) {
        int nWorkers = 10;

        // Check state for each worker
        for (int i = 0; i < nWorkers; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);

            uint8_t count = getCounter(memory, workerKey);
            if (count < 1) {
                return false;
            }
        }

        return true;
    }

    int exec(FaasmMemory *memory) {
        // Get current epoch count
        uint8_t thisEpoch = getCounter(memory, EPOCH_COUNT_KEY);

        // Resubmit if not yet finished
        bool workersFinished = checkWorkers(memory);
        if (!workersFinished) {
            // Recursive call
            uint8_t barrierInput[1] = {0};
            memory->chainFunction("sgd_barrier", barrierInput, 1);
            return 0;
        }

        // Work out difference between weights
        int nWeights = 10;
        MatrixXd weights = faasm::readMatrixFromState(memory, WEIGHTS_KEY, 1, nWeights);

        MatrixXd realWeights = faasm::readMatrixFromState(memory, REAL_WEIGHTS_KEY, 1, nWeights);

        double mse = faasm::calculateMse(realWeights, weights);

        // Drop out if finished
        uint8_t maxEpochs = 10;
        if (thisEpoch >= maxEpochs) {
            printf("SGD complete over %i epochs (MSE = %f)\n", thisEpoch, mse);
            return 0;
        }

        // Increment epoch counter
        incrementCounter(memory, EPOCH_COUNT_KEY);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (MSE = %f)\n", nextEpoch, mse);

        // Load inputs
        int nTrain = 1000;
        MatrixXd inputs = faasm::readMatrixFromState(memory, INPUTS_KEY, nWeights, nTrain);

        // Shuffle and update
        faasm::shuffleMatrixColumns(inputs);
        writeMatrixState(memory, INPUTS_KEY, inputs);

        // Kick off next epoch
        uint8_t epochInput[1] = {0};
        memory->chainFunction("sgd_epoch", epochInput, 1);

        return 0;
    }
}