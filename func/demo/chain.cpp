#include <faasm/faasm.h>
#include <stdio.h>
#include <vector>

size_t dataLength = 3 * sizeof(int);

int checkChained(int funcIdx) {
    // Read input
    uint8_t inputBuf[dataLength];
    faasmGetInput(inputBuf, dataLength);
    int *intInput = (int *) inputBuf;

    std::vector<int> actual(intInput, intInput + (3 * sizeof(int)));

    // Check vs expectation
    std::vector<int> expected(3);
    expected[0] = funcIdx;
    expected[1] = funcIdx + 1;
    expected[2] = funcIdx + 2;

    if(actual != expected) {
        printf("ERROR - chained input does not match expected (actual=[%i, %i, %i], expected=[%i, %i, %i])\n",
                actual[0], actual[1], actual[2], expected[0], expected[1], expected[2]);
        return -1;
    }

    return 0;
}

/**
 * Passes some inputs onto other functions
 */
FAASM_MAIN_FUNC() {
    int nCalls = 3;
    int callIds[nCalls];

    // Dispatch chained calls in a loop
    for (int i = 0; i < nCalls; i++) {
        int funcData[3];
        funcData[0] = i;
        funcData[1] = i + 1;
        funcData[2] = i + 2;

        int funcIdx = i + 1;

        // Chain the call
        int callId = faasmChainThisInput(funcIdx, (uint8_t *) funcData, dataLength);
        callIds[i] = callId;
    }

    // Wait for calls to finish
    for(int callId : callIds) {
        int result = faasmAwaitCall(callId);
        if(result != 0) {
            printf("Chained call %i failed\n", callId);
        }
    }

    return 0;
}

FAASM_FUNC(chainOne, 1) {
    return checkChained(1);
}

FAASM_FUNC(chainTwo, 2) {
    return checkChained(2);
}

FAASM_FUNC(chainThree, 3) {
    return checkChained(3);
}