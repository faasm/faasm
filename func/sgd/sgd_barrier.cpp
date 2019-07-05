#include "faasm/time.h"
#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/print.h"
#include "faasm/sgd.h"
#include <unistd.h>

using namespace faasm;

FAASM_MAIN_FUNC() {
    bool fullAsync = getEnvFullAsync();
    SgdParams p = readParamsFromState(PARAMS_KEY, fullAsync);

    // Get epoch count and barrier count(starts at zero)
    int thisEpoch = getCounter(EPOCH_COUNT_KEY, p.fullAsync);
    int barrierCount = getCounter(BARRIER_COUNT_KEY, p.fullAsync);

    // See if we've finished the epoch
    bool isFinished = readEpochFinished(p);
    if (!isFinished) {
        if (barrierCount > MAX_BARRIER_COUNT) {
            printf("Exceeded max barrier count. Stopping.\n");
            return 0;
        }

        // Increment the barrier count
        faasm::incrementCounter(BARRIER_COUNT_KEY, p.fullAsync);

        // Try again
        printf("Epoch %i not finished (barrier count %i)\n", thisEpoch, barrierCount);
        faasmChainFunction("sgd_barrier");
        return 0;
    }

    long tsTotal = p.nEpochs * sizeof(double);
    long tsOffset = thisEpoch * sizeof(double);

    // Record the time for this epoch
    double ts = faasm::getSecondsSinceEpoch();
    auto tsBytes = reinterpret_cast<uint8_t *>(&ts);
    faasmWriteStateOffset(LOSS_TIMESTAMPS_KEY, tsTotal, tsOffset, tsBytes, sizeof(double), p.fullAsync);

    // Record the loss for this epoch
    double loss = faasm::readRootMeanSquaredError(p);
    auto lossBytes = reinterpret_cast<uint8_t *>(&loss);
    faasmWriteStateOffset(LOSSES_KEY, tsTotal, tsOffset, lossBytes, sizeof(double), p.fullAsync);

    // Drop out if finished all epochs
    if (thisEpoch >= p.nEpochs - 1) {
        char *tsString = faasm::floatToStr(ts);
        char *lossString = faasm::floatToStr(ts);
        printf("SGD complete over %i epochs (time = %s, loss = %s)\n", thisEpoch, tsString, lossString);
        return 0;
    }

    // Decay learning rate (apparently hogwild doesn't actually do this although it takes in the param)
    // p.learningRate = p.learningRate * p.learningDecay;
    // writeParamsToState(PARAMS_KEY, p);

    // Increment epoch counter
    incrementCounter(EPOCH_COUNT_KEY, p.fullAsync);
    int nextEpoch = thisEpoch + 1;
    char *tsString = faasm::floatToStr(ts);
    char *lossString = faasm::floatToStr(loss);
    printf("Starting epoch %i (time = %s, loss = %s)\n", nextEpoch, tsString, lossString);

    // Kick off next epoch
    faasmChainFunction("sgd_epoch");

    return 0;
}