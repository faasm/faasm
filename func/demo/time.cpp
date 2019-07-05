#include "faasm/faasm.h"
#include "faasm/time.h"

#include <stdio.h>

FAASM_MAIN_FUNC() {
    double secs = faasm::getSecondsSinceEpoch();

    size_t strLen = 8 + sizeof(double);
    char str[strLen];
    sprintf(str, "Seconds: %.3f", secs);

    auto output = reinterpret_cast<uint8_t *>(str);
    faasmSetOutput(output, strLen);

    return 0;
}