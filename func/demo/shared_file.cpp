#include "faasm/faasm.h"
#include "faasm/files.h"

#include <string>
#include <stdio.h>

/**
 * Test reading a shared file
 */
FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();
    auto inputBuffer = new uint8_t[inputSize];
    faasmGetInput(inputBuffer, inputSize);

    // Open the file
    auto inputPath = reinterpret_cast<char *>(inputBuffer);
    char *content = faasm::readFileToString(inputPath);
    if (content == nullptr) {
        printf("Failed to open file at %s\n", inputPath);
        return 1;
    }

    // Write file content as output
    long length = faasm::getFileLength(inputPath);
    faasmSetOutput(reinterpret_cast<uint8_t *>(content), length);

    return 0;
}
