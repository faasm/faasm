#include "fake.h"

#include <stdio.h>

/**
 * Self-contained function to test taking in arguments
 */
int mult(int a, int b) {
    int result = a * b;
    printf("Multiplying %i and %i to get %i\n", a, b, result);

    // Call the reverse function back from the implementer
    multReverse(a, b);

    return result;
}

int main() {
    // Do nothing
    return 0;
}
