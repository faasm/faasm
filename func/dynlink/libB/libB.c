#include "libB.h"
#include <sharedHeader.h>

#include <stdio.h>

int divide(int a, int b)
{
    int result = a / b;
    printf("Dividing %i and %i to get %i\n", a, b, result);

    return result;
}

int divideGlobal()
{
    printf("Dividing struct with name: %s\n", sharedStructInstance.name);
    return divide(sharedStructInstance.alpha, sharedStructInstance.beta);
}

int invokeSharedFunc()
{
    // Invoking the function in this roundabout way is necessary to force using
    // the GOT
    sharedStructInstance.func = sharedFuncInstance;

    int i = 4;
    int result = sharedStructInstance.func(i);

    printf("Got result from shared func: %i -> %i\n", i, result);

    return result;
}
