#include "faasm/time.h"
#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"
#include <unistd.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Get current epoch count (starts at zero)
        int thisEpoch = getCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);

        // See if we've finished the epoch
        bool isFinished = readEpochFinished(memory, p);
        if (!isFinished) {
            // Try again
            memory->chainFunction("sgd_barrier");
            return 0;
        }

        // Record the time for this epoch
        long ts = faasm::getMillisSinceEpoch();
        long tsTotal = p.nEpochs * sizeof(double);
        long tsOffset = thisEpoch * sizeof(long);
        auto tsBytes = reinterpret_cast<uint8_t *>(&ts);
        memory->writeStateOffset(LOSS_TIMESTAMPS_KEY, tsTotal, tsOffset, tsBytes, sizeof(long), p.fullAsync);

        // Record the loss for this epoch
        double loss = faasm::readRootMeanSquaredError(memory, p);
        long lossTotal = p.nEpochs * sizeof(double);
        long lossOffset = thisEpoch * sizeof(double);
        auto lossBytes = reinterpret_cast<uint8_t *>(&loss);
        memory->writeStateOffset(LOSSES_KEY, lossTotal, lossOffset, lossBytes, sizeof(double), p.fullAsync);

        // Drop out if finished all epochs
        if (thisEpoch >= p.nEpochs - 1) {
            printf("SGD complete over %i epochs (loss = %f)\n", thisEpoch, loss);
            return 0;
        }

        // Increment epoch counter
        incrementCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (MSE = %f)\n", nextEpoch, loss);

        // Kick off next epoch
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}