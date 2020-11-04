#include <cstdio>
#include <faasm/faasm.h>
#include <vector>

#define STATE_KEY "size_check"

FAASM_MAIN_FUNC()
{
    size_t initialSize = faasmReadStateSize(STATE_KEY);
    if (initialSize > 0) {
        printf("Expected state to be zero size, got %li\n", initialSize);
        return 1;
    }

    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faasmWriteState(STATE_KEY, bytes.data(), bytes.size());

    size_t otherSize = faasmReadStateSize(STATE_KEY);
    if (otherSize != bytes.size()) {
        printf(
          "Expected state size %li but got %li\n", bytes.size(), otherSize);
        return 1;
    }

    return 0;
}
