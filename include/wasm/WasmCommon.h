#pragma once

/* The constants defined in this header file are meant to
 * be shared across all supported WASM runtimes: WAVM, WAMR, and WAMR inside
 * SGX. In particular, this header must also be included inside an SGX enclave
 * so we want to keep the external includes to a minimum. Some of the values for
 * this constants must match those used when cross-compiling WASM modules, as
 * a consequence, we define them in faasmtools/build.py and populate the
 * values using CMake
 */

#define WASM_BYTES_PER_PAGE 65536

#define ONE_KB_BYTES 1024
#define ONE_MB_BYTES 1024 * 1024

// Note: this is *not* controlling the size provisioned by the linker, that is
// hard-coded in the build. This variable is just here for reference and must be
// updated to match the value in the build.
#define STACK_SIZE 4194304
#define THREAD_STACK_SIZE (STACK_SIZE / 2)

// Properties of dynamic modules. Heap size must be wasm-module-page-aligned.
// One page is 64kB
#define DYNAMIC_MODULE_STACK_SIZE (2 * ONE_MB_BYTES)
#define DYNAMIC_MODULE_MEMORY_SIZE (66 * WASM_BYTES_PER_PAGE)
#define GUARD_REGION_SIZE (10 * WASM_BYTES_PER_PAGE)

// Special known function names
// Zygote function (must match faasm.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_faasm_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

#define MAX_WASM_MEM 4294901760
#define MAX_WASM_MEMORY_PAGES (MAX_WASM_MEM / WASM_BYTES_PER_PAGE)
#define MAX_TABLE_SIZE 500000

