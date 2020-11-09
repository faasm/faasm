#include <faasm/faasm.h>
#include <faasm/input.h>
#include <stdio.h>
#include <vector>

#define APPEND_KEY "dummy_append"

int appender()
{
    long appendCount = faasm::getTypedInput<long>(0);
    if (appendCount == 0) {
        printf("Append count was zero\n");
        return 1;
    }

    for (long i = 0; i < appendCount; i++) {
        faasmAppendState(APPEND_KEY, BYTES(&i), sizeof(long));
    }

    return 0;
}

FAASM_MAIN_FUNC()
{
    // Chain the calls to do the appending
    long appendCountA = 100;
    long appendCountB = 200;

    unsigned int callIdA =
      faasmChain(appender, (uint8_t*)&appendCountA, sizeof(long));
    unsigned int callIdB =
      faasmChain(appender, (uint8_t*)&appendCountB, sizeof(long));

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
    faasmReadAppendedState(APPEND_KEY, (uint8_t*)buffer, buffSize, nElems);

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
        printf("Expected sum and actual don't match (%li != %li)\n",
               expectedSum,
               actualSum);
        return 1;
    }

    delete[] buffer;
    return 0;
}

