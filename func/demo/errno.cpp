#include "faasm/faasm.h"

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

FAASM_MAIN_FUNC() {
    // Do something to set errno (e.g. stat a non-existent file)
    struct stat64 s{};
    int result = stat64("foobar/baz", &s);
    int actual = errno;

    if (result >= 0) {
        printf("Expected stat to fail but got %i\n", result);
    }

    if (actual != ENOENT) {
        printf("Expected errno to be %i, but got %i\n", ENOENT, actual);
        return 1;
    }

    return 0;
}
