#pragma once

// Minimum stack size
#define FAASM_MIN_STACK_SIZE_MB 16
#define FAASM_MIN_STACK_SIZE_BYTES (FAASM_MIN_STACK_SIZE_MB * 1024L * 1024L)

namespace isolation {
void checkStackSize();
}
