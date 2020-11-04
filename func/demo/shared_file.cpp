#include "faasm/faasm.h"
#include "faasm/files.h"
#include "faasm/input.h"

#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

/**
 * Test reading a shared file
 */
FAASM_MAIN_FUNC()
{
    const char* inputStr = faasm::getStringInput("");

    // First stat the file to make sure it exists
    struct stat64 res
    {};
    int statRes = stat64(inputStr, &res);
    if (statRes != 0) {
        printf("Failed to stat shared file: %s\n", strerror(errno));
        return 1;
    }

    // Open the file
    char* content = faasm::readFileToString(inputStr);
    if (content == nullptr) {
        printf("Failed to open file at %s\n", inputStr);
        return 1;
    }

    // Write file content as output
    long length = faasm::getFileLength(inputStr);
    faasmSetOutput(BYTES(content), length);

    return 0;
}
