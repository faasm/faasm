#include "faasm/faasm.h"
#include "faasm/files.h"
#include "faasm/input.h"

#include <stdio.h>

/**
 * Test reading a shared file
 */
FAASM_MAIN_FUNC() {
    const char* inputStr = faasm::getStringInput("");

    // Open the file
    char *content = faasm::readFileToString(inputStr);
    if (content == nullptr) {
        printf("Failed to open file at %s\n", inputStr);
        return 1;
    }

    // Write file content as output
    long length = faasm::getFileLength(inputStr);
    faasmSetOutput(reinterpret_cast<uint8_t *>(content), length);

    return 0;
}
