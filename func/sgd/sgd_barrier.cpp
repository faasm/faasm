#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Get current epoch count
        uint8_t thisEpoch = getCounter(memory, EPOCH_COUNT_KEY);

        // Iterate through batches and check their errors
        double totalError = 0.0;
        for (int i = 0; i < p.nBatches; i++) {
            char batchKey[10];
            sprintf(batchKey, "batch-%i", i);

            double error = 0.0;
            auto errorBytes = reinterpret_cast<uint8_t *>(&error);
            memory->readState(batchKey, errorBytes, sizeof(double));

            // If error is still zero, we've not yet finished
            if (error == 0.0) {
                // Recursive call to barrier
                memory->chainFunction("sgd_barrier");
                return 0;
            }

            totalError += error;
        }

        // Calculate the mean squared error across all batches
        double mse = totalError / p.nTrain;

        // Drop out if finished
        if (thisEpoch >= p.maxEpochs) {
            printf("SGD complete over %i epochs (MSE = %f)\n", thisEpoch, mse);
            return 0;
        }

        // Increment epoch counter
        incrementCounter(memory, EPOCH_COUNT_KEY);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (MSE = %f)\n", nextEpoch, mse);

        // Kick off next epoch
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}