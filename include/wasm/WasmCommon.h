#pragma once

#include <cstdint>
#include <cstdio>

/* The constants and helper methods defined in this header file are meant to
 * be shared across all supported WASM runtimes: WAVM, WAMR, and WAMR inside
 * SGX. In particular, this header must also be included inside an SGX enclave
 * so we want to keep the external includes to a minimum.
 */

#define WASM_BYTES_PER_PAGE 65536

#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is
// hard-coded in the build. This variable is just here for reference and must be
// updated to match the value in the build.
// TODO(csegarragonz): read this from the environment
#define STACK_SIZE (4 * ONE_MB_BYTES)
#define THREAD_STACK_SIZE (2 * ONE_MB_BYTES)

// Properties of dynamic modules. Heap size must be wasm-module-page-aligned.
// One page is 64kB
#define DYNAMIC_MODULE_STACK_SIZE (2 * ONE_MB_BYTES)
#define DYNAMIC_MODULE_MEMORY_SIZE (66 * WASM_BYTES_PER_PAGE)
#define GUARD_REGION_SIZE (10 * WASM_BYTES_PER_PAGE)

// Special known function names
// Zygote function (must match faasm.h linked into the functions themselves)
// TODO(csegarragonz): read this from the environment
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

// TODO(csegarragonz): read this from the environment
// TODO: also, this number is not correct
#define MAX_WASM_MEM (1024L * 1024L * 1024L * 4L)

namespace wasm {

inline bool isWasmPageAligned(int32_t offset)
{
    return !(offset & (WASM_BYTES_PER_PAGE - 1));
}

inline size_t getNumberOfWasmPagesForBytes(size_t nBytes)
{
    // Round up to nearest page
    size_t pageCount =
      (size_t(nBytes) + WASM_BYTES_PER_PAGE - 1) / WASM_BYTES_PER_PAGE;

    return pageCount;
}
}
