#include <faasm/faasm.h>
#include <stdio.h>
#include <vector>
#include <faasm/input.h>

#define APPEND_KEY "dummy_append"


FAASM_MAIN_FUNC() {
    // Chain the calls to do the appending
    long appendCountA = 100;
    long appendCountB = 200;

    unsigned int callIdA = faasmChainThisInput(1, (uint8_t *) &appendCountA, sizeof(long));
    unsigned int callIdB = faasmChainThisInput(1, (uint8_t *) &appendCountB, sizeof(long));

    // Wait for calls to finish
    unsigned int resultA = faasmAwaitCall(callIdA);
    unsigned int resultB = faasmAwaitCall(callIdB);

    if (resultA != 0 || resultB != 0) {
        printf("Chained call failed\n");
        return 1;
    } else {
        printf("Chained calls succeeded\n");
    }

    // Read in the results
    long nElems = appendCountA + appendCountB;
    size_t buffSize = nElems * sizeof(long);
    auto buffer = new long[nElems];
    faasmReadAppendedState(APPEND_KEY, (uint8_t *) buffer, buffSize, nElems);

    // Work out the expectation
    long expectedSum = 0;
    for (long i = 0; i < appendCountA; i++) {
        expectedSum += i;
    }
    for (long i = 0; i < appendCountB; i++) {
        expectedSum += i;
    }

    // Work out the actual
    long actualSum = 0;
    for (long i = 0; i < nElems; i++) {
        actualSum += buffer[i];
    }

    if (expectedSum != actualSum) {
        printf("Expected sum and actual don't match (%li != %li)\n", expectedSum, actualSum);
        return 1;
    }

    delete[] buffer;
    return 0;
}

FAASM_FUNC(appender, 1) {
    long appendCount = faasm::getTypedInput<long>(0);
    if (appendCount == 0) {
        printf("Append count was zero\n");
        return 1;
    }

    for (long i = 0; i < appendCount; i++) {
        faasmAppendState(APPEND_KEY, reinterpret_cast<uint8_t *>(&i), sizeof(long));
    }

    return 0;
}