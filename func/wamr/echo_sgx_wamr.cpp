//Modified the original func/demo/echo.cpp
//#include "faasm/faasm.h"
//#include "faasm/input.h"
#include <sgx/faasm_sgx_wamr.h>
#include <stdio.h>
#include <string.h>

/**
 * Writes the input to the output
 */
FAASM_MAIN() {
    uint32_t input_size = faasmGetInputSize();
    uint8_t input[input_size];
    faasmGetInput(input, input_size);
    printf("Input: %s (size: %u)\n",(char*)input, input_size);
    //const char* inputStr = faasm::getStringInput("");
    //size_t inputLen = strlen(inputStr);
    // Handle empty input
    /*if (inputLen == 0) {
        const char *output = "Nothing to echo";
        auto bytesOutput = reinterpret_cast<const uint8_t *>(output);
        faasmSetOutput(bytesOutput, strlen(output));
    }*/
    printf("Echoing %s\n", (char*)input);
    faasmSetOutput(input, input_size);
}
