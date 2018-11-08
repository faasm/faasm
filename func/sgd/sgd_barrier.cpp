#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Get current epoch count
        uint8_t thisEpoch = getCounter(memory, EPOCH_COUNT_KEY);

        double mse = faasm::readRootMeanSquaredError(memory, p);

        // If error is still zero, we've not yet finished
        if (mse == 0.0) {
            // Recursive call to barrier
            memory->chainFunction("sgd_barrier");
            return 0;
        }

        // Drop out if finished all epochs
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