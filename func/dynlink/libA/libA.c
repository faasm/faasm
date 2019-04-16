#include "libA.h"
#include "sharedHeader.h"

#include <stdio.h>

SharedStruct sharedStructInstance = {
        "Shared struct",
        10,
        5,
        0   // Func pointer
};

int sharedFuncInstance(int a) {
    return a * a;
}

int multiply(int a, int b) {
    int result = a * b;
    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}

int multiplyGlobal() {
    printf("Multiplying struct with name: %s\n", sharedStructInstance.name);
    return sharedStructInstance.alpha * sharedStructInstance.beta;
}
