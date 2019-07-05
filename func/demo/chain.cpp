#include <faasm/faasm.h>
#include <stdio.h>

int dataLength = 3 * sizeof(int);

void printChained(const char *name) {
    uint8_t inputBuf[dataLength];
    faasmGetInput(inputBuf, dataLength);

    int *intInput = (int *) inputBuf;

    printf("CHAIN %s: {%i, %i, %i}\n", name, intInput[0], intInput[1], intInput[2]);
}

/**
 * Passes some inputs onto other functions
 */
FAASM_MAIN_FUNC() {
    int nCalls = 3;
    int callIds[nCalls];
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
        faasmAwaitCall(callId);
    }

    return 0;
}

FAASM_FUNC(chainOne, 1) {
    printChained("ONE");
    return 0;
}

FAASM_FUNC(chainTwo, 2) {
    printChained("TWO");
    return 0;
}

FAASM_FUNC(chainThree, 3) {
    printChained("THREE");
    return 0;
}