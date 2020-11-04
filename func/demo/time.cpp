#include "faasm/time.h"
#include "faasm/faasm.h"

#include <stdio.h>

FAASM_MAIN_FUNC()
{
    double secs = faasm::getSecondsSinceEpoch();

    size_t strLen = 8 + sizeof(double);
    char str[strLen];
    sprintf(str, "Seconds: %.3f", secs);

    auto output = BYTES(str);
    faasmSetOutput(output, strLen);

    return 0;
}
