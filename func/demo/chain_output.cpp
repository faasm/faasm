#include <faasm/compare.h>
#include <faasm/faasm.h>
#include <stdio.h>

int otherA()
{
    uint8_t outputA[5] = { 0, 1, 2, 3, 4 };
    faasmSetOutput(outputA, 5);
    return 0;
}

int otherB()
{
    uint8_t outputB[3] = { 5, 4, 3 };
    faasmSetOutput(outputB, 3);
    return 0;
}

/**
 * Checks getting output from chained functions
 */
int main(int argc, char* argv[])
{
    unsigned int callIdA = faasmChain(otherA, nullptr, 0);
    unsigned int callIdB = faasmChain(otherB, nullptr, 0);

    uint8_t expectedA[5] = { 0, 1, 2, 3, 4 };
    uint8_t actualA[5] = { 0, 0, 0, 0, 0 };

    uint8_t expectedB[3] = { 5, 4, 3 };
    uint8_t actualB[3] = { 0, 0, 0 };

    unsigned int resA = faasmAwaitCallOutput(callIdA, actualA, 5);
    unsigned int resB = faasmAwaitCallOutput(callIdB, actualB, 3);

    if (resA != 0 || resB != 0) {
        printf("One or more chained calls failed: %i %i\n", resA, resB);
        return 1;
    }

    if (!faasm::compareArrays<uint8_t>(actualA, expectedA, 5)) {
        return 1;
    }

    if (!faasm::compareArrays<uint8_t>(actualB, expectedB, 3)) {
        return 1;
    }

    printf("Chained outputs as expected\n");

    return 0;
}
