#include <faasm/faasm.h>
#include <stdio.h>

#define INPUT_VAL 42

int chainOne()
{
    int input = 0;
    faasmGetInput(BYTES(&input), sizeof(int));

    if (input == INPUT_VAL) {
        printf("Got expected chained input of %i\n", input);
        return 0;
    } else {
        printf("Got unexpected chained input of %i\n", input);
        return 1;
    }
}

FAASM_MAIN_FUNC()
{
    // Make one chained call
    int inputVal = INPUT_VAL;
    unsigned int callId =
      faasmChain(chainOne, (uint8_t*)&inputVal, sizeof(int));

    unsigned int result = faasmAwaitCall(callId);

    if (result != 0) {
        printf("Chained call failed\n");
        return 1;
    } else {
        printf("Chained call succeeded\n");
    }

    return 0;
}
