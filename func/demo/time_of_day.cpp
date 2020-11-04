#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/time.h>

FAASM_MAIN_FUNC()
{
    timeval tv{};
    gettimeofday(&tv, nullptr);

    size_t strLen = 8 + sizeof(long int);
    char str[strLen];
    sprintf(str, "Seconds: %li", tv.tv_sec);

    auto output = BYTES(str);
    faasmSetOutput(output, strLen);

    return 0;
}
