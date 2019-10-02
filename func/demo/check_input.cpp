#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>

FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();
    auto inputBuffer = new uint8_t[inputSize];
    faasmGetInput(inputBuffer, inputSize);

    auto inputStr = reinterpret_cast<char *>(inputBuffer);
    const char* expected = "http://www.foobar.com";
    if(strcmp(inputStr, expected) != 0) {
        printf("Expected %s but got %s\n", expected, inputStr);
        return 1;
    } else {
        printf("Expected %s and got %s\n", expected, inputStr);
        return 0;
    }
}
