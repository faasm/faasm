#include "faasm/time.h"
#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"
#include <unistd.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);

        // Get current epoch count (starts at zero)
        int thisEpoch = getCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);

        // See if we've finished the epoch
        bool isFinished = readEpochFinished(memory, p);
        if (!isFinished) {
            // Try again
            memory->chainFunction("sgd_barrier");
            return 0;
        }

        long tsTotal = p.nEpochs * sizeof(double);
        long tsOffset = thisEpoch * sizeof(double);

        // Record the time for this epoch
        double ts = faasm::getSecondsSinceEpoch();
        auto tsBytes = reinterpret_cast<uint8_t *>(&ts);
        memory->writeStateOffset(LOSS_TIMESTAMPS_KEY, tsTotal, tsOffset, tsBytes, sizeof(double), p.fullAsync);

        // Record the loss for this epoch
        double loss = faasm::readRootMeanSquaredError(memory, p);
        auto lossBytes = reinterpret_cast<uint8_t *>(&loss);
        memory->writeStateOffset(LOSSES_KEY, tsTotal, tsOffset, lossBytes, sizeof(double), p.fullAsync);

        // Drop out if finished all epochs
        if (thisEpoch >= p.nEpochs - 1) {
            printf("SGD complete over %i epochs (time = %f, loss = %f)\n", thisEpoch, ts, loss);
            return 0;
        }

        // Decay learning rate
        p.learningRate = p.learningRate * p.learningDecay;
        writeParamsToState(memory, PARAMS_KEY, p);

        // Increment epoch counter
        incrementCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (time = %f, loss = %f)\n", nextEpoch, ts, loss);

        // Kick off next epoch
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}