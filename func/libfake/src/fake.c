#include "fake.h"

#include <stdio.h>

int doubleInt(int arg) {
    int result = 2*arg;

    printf("Doubling %i to %i\n",arg, result);

    return result;
}
