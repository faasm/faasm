#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Get current epoch count (starts at zero)
        uint8_t thisEpoch = getCounter(memory, EPOCH_COUNT_KEY);

        // See if we've finished the epoch
        bool isFinished = readEpochFinished(memory, p);
        if (!isFinished) {
            // Try again
            memory->chainFunction("sgd_barrier");
            return 0;
        }

        // Calculate the error
        double rmse = faasm::readRootMeanSquaredError(memory, p);

        // Record the loss for this epoch
        long offset = thisEpoch * sizeof(double);
        auto rmseBytes = reinterpret_cast<uint8_t *>(&rmse);
        memory->writeStateOffset(LOSSES_KEY, offset, rmseBytes, sizeof(double));

        // Drop out if finished all epochs
        if (thisEpoch >= p.nEpochs - 1) {
            printf("SGD complete over %i epochs (MSE = %f)\n", thisEpoch, rmse);
            return 0;
        }

        // Increment epoch counter
        incrementCounter(memory, EPOCH_COUNT_KEY);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (MSE = %f)\n", nextEpoch, rmse);

        // Kick off next epoch
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}