#include "faasm/core.h"
#include <faasm/faasm.h>
#include <stdio.h>
#include <vector>

size_t dataLength = 3 * sizeof(int);

int checkChained(int funcIdx)
{
    // Read input
    uint8_t inputBuf[dataLength];
    faasmGetInput(inputBuf, dataLength);
    int* intInput = reinterpret_cast<int*>(inputBuf);

    std::vector<int> actual(intInput, intInput + 3);

    // Check vs expectation
    std::vector<int> expected(3);
    expected[0] = funcIdx;
    expected[1] = funcIdx + 1;
    expected[2] = funcIdx + 2;

    if (actual != expected) {
        printf("ERROR - chained input does not match expected (actual=[%i, %i, "
               "%i], expected=[%i, %i, %i])\n",
               actual[0],
               actual[1],
               actual[2],
               expected[0],
               expected[1],
               expected[2]);
        return -1;
    }

    return 0;
}

int chainOne()
{
    return checkChained(1);
}

int chainTwo()
{
    return checkChained(2);
}

int chainForwardNested()
{
    return checkChained(4);
}

int chainForward()
{
    // Check input to this function as expected
    checkChained(3);

    // Make and await another chained call
    int funcData[3];
    funcData[0] = 4;
    funcData[1] = 5;
    funcData[2] = 6;

    auto inputBytes = (uint8_t*)funcData;
    unsigned int callId =
      faasmChain(chainForwardNested, inputBytes, dataLength);

    // Await and check the nested call
    unsigned int result = faasmAwaitCall(callId);

    if (result != 0) {
        printf("Nested chained call failed\n");
        return 1;
    } else {
        printf("Nested chained call succeeded\n");
    }

    return 0;
}

/**
 * Passes some inputs onto other functions
 */
FAASM_MAIN_FUNC()
{
    int nCalls = 3;
    int callIds[nCalls];
    FaasmFuncPtr funcs[3] = { chainOne, chainTwo, chainForward };

    // Dispatch chained calls in a loop
    for (int i = 0; i < nCalls; i++) {
        int funcIdx = i + 1;

        // Set up some input data to be checked
        int funcData[3];
        funcData[0] = funcIdx;
        funcData[1] = funcIdx + 1;
        funcData[2] = funcIdx + 2;

        // Chain the call
        FaasmFuncPtr func = funcs[i];
        unsigned int callId = faasmChain(func, (uint8_t*)funcData, dataLength);
        callIds[i] = callId;
    }

    // Wait for calls to finish
    for (int callId : callIds) {
        unsigned int result = faasmAwaitCall(callId);

        if (result != 0) {
            printf("Chained call failed\n");
            return 1;
        } else {
            printf("Chained call succeeded\n");
        }
    }

    return 0;
}
