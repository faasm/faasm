#include "fake.h"

#include <stdio.h>

int mult(int a, int b) {
    int result = a * b;

    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}
