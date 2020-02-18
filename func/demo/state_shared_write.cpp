#include <faasm/faasm.h>
#include <stdio.h>
#include <vector>

FAASM_MAIN_FUNC() {
    const char *key = "state_shared_example";

    // Write an empty value
    std::vector<uint8_t> bytes = {0, 0, 0, 0, 0, 0, 0};
    unsigned long bytesLen = bytes.size();
    faasmWriteState(key, bytes.data(), bytesLen);

    // Get two pointers to the memory
    uint8_t *actualA = faasmReadStatePtr(key, bytesLen);
    uint8_t *actualB = faasmReadStatePtr(key, bytesLen);

    // Write to one pointer
    uint8_t newValue = 5;
    for (unsigned long i = 0; i < bytesLen; i++) {
        actualA[i] = newValue;
    }

    // Check update seen by other
    bool success = actualA == actualB;
    for (unsigned long i = 0; i < bytesLen; i++) {
        success &= (actualB[i] == newValue);
    }

    // Check that pointers are also equal
    faasmSetOutput(BYTES(&success), 1);

    return 0;
}