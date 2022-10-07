#pragma once

#include <cstdint>

#define WASM_BYTES_PER_PAGE 65536

#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is
// hard-coded in the build. This variable is just here for reference and must be
// updated to match the value in the build.
#define STACK_SIZE (4 * ONE_MB_BYTES)
#define THREAD_STACK_SIZE (2 * ONE_MB_BYTES)

// Properties of dynamic modules. Heap size must be wasm-module-page-aligned.
// One page is 64kB
#define DYNAMIC_MODULE_STACK_SIZE (2 * ONE_MB_BYTES)
#define DYNAMIC_MODULE_MEMORY_SIZE (66 * WASM_BYTES_PER_PAGE)
#define GUARD_REGION_SIZE (10 * WASM_BYTES_PER_PAGE)

namespace wasm {

inline bool isWasmPageAligned(int32_t offset)
{
    return !(offset & (WASM_BYTES_PER_PAGE - 1));
}
}
