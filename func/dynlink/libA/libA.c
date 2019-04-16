#include "libA.h"
#include "sharedHeader.h"

#include <stdio.h>

SharedStruct SharedStructInstance = {
        "Shared struct",
        10,
        5
};

int multiply(int a, int b) {
    int result = a * b;
    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}

int multiplyGlobal() {
    return SharedStructInstance.alpha * SharedStructInstance.beta;
}
