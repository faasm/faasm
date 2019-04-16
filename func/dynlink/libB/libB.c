#include "libB.h"
#include <sharedHeader.h>

#include <stdio.h>

int divide(int a, int b) {
    int result = a/b;
    printf("Dividing %i and %i to get %i\n", a, b, result);

    return result;
}

int divideGlobal() {
    printf("Global struct name: %s", SharedStructInstance.name);
    return divide(SharedStructInstance.alpha, SharedStructInstance.beta);
}
