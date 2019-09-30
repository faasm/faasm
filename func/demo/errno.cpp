#include "faasm/faasm.h"

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

FAASM_MAIN_FUNC() {
    int *errnoLoc = __errno_location();

#ifdef __wasm__
    // Print out errno address
    printf("Errno location = %i\n", errnoLoc);
#endif
    printf("Errno initial value = %i\n", *errnoLoc);

    // Do something to set errno (e.g. stat a non-existent file)
    struct stat64 s{};
    int result = stat64("foobar/baz", &s);

    if (result != -1) {
        printf("Expected stat to fail but got %i\n", result);
        return 1;
    }

    if(*errnoLoc != errno) {
        printf("Dereferencing errno location gives %i, but macro gives %i\n", *errnoLoc, errno);
        return 1;
    }

    if (errno != ENOENT) {
        printf("Expected errno to be %i, but got %i\n", ENOENT, errno);
        return 1;
    }

    // Do something else to set a different errno and check
    // First of all check we can't get an invalid clock
    timespec ts{};
    int retVal = clock_gettime(12345, &ts);
    if(retVal != -1) {
        printf("Expected invalid clock to fail (but ret val = %i)\n", retVal);
        return 1;
    }

    if(errno != EINVAL) {
        printf("Expected invalid clock to give errno %i but was %i)\n", EINVAL, errno);
        return 1;
    }

    return 0;
}
