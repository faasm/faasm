#include "fake.h"

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

int mult(int a, int b) {
    int result = a * b;

    getuid();

    printf("Multiplying %i and %i to get %i\n", a, b, result);

    return result;
}

int main() {
    // Do nothing
    return 0;
}
