#include "faasm/faasm.h"

#include <string>
#include <unistd.h>

FAASM_MAIN_FUNC()
{
    int actual = sysconf(_SC_NPROCESSORS_ONLN);
    int expected = 1;

    if (actual != expected) {
        printf("Number processors not as expected (expected %i but was %i)\n",
               expected,
               actual);
        return 1;
    }

    return 0;
}
