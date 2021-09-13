# WARNING: the CMake flags and pre-processor definitions that we set in this
# file are carefully chosen for the WAMR build to run. Adding or removing flags
# may break Faasm.

# Set target platform details
set (WAMR_BUILD_PLATFORM "linux")
set (WAMR_BUILD_TARGET X86_64)
set (WAMR_BUILD_SPEC_TEST 0)

# Set AOT mode and JIT for code generation
set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_JIT 1)
set (WAMR_BUILD_LAZY_JIT 0)

# Set libraries
set (WAMR_BUILD_LIBC_BUILTIN 1)
set (WAMR_BUILD_LIBC_WASI 1)
set (WAMR_BUILD_LIB_PTHREAD 0)

# WAMR features
set (WAMR_BUILD_SIMD 0)
set (WAMR_BUILD_MULTI_MODULE 1)
set (WAMR_BUILD_BULK_MEMORY 0)
set (WAMR_DISABLE_HW_BOUND_CHECK 1)
# This definition prevents a buffer underflow during code generation
add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)

include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
# 13/09/2021 - Setting the CMake flag includes the code required to run
# code generation, but the pre-processor macro introduces LLVM errors. It is
# safe to disable as we actually don't run JITed code.
remove_definitions("-DWASM_ENABLE_JIT=1")
faasm_private_lib(wamrlib "${WAMR_RUNTIME_LIB_SOURCE}")

# -----------------------------
# Faasm-specific configuration
# -----------------------------

target_include_directories(wamrlib PUBLIC
    ${PLATFORM_SHARED_DIR}
)

set(FAASM_WAMR_HEADERS
    "${FAASM_INCLUDE_DIR}/wamr/native.h"
    "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
)

set(FAASM_WAMR_SRC
    WAMRWasmModule.cpp
    codegen.cpp
    dynlink.cpp
    env.cpp
    filesystem.cpp
    funcs.cpp
    memory.cpp
    native.cpp
    pthread.cpp
    state.cpp
    stubs.cpp
    timing.cpp
    ${FAASM_WAMR_HEADERS}
)

# Link the specific LLVM libraries that WAMR needs
llvm_map_components_to_libnames(
    WAMR_LLVM_LIBRARIES
    core
    lto
    executionengine
    mcjit
)

# Link everything together
faasm_private_lib(wamrmodule "${FAASM_WAMR_SRC}")
target_link_libraries(wamrmodule
    wasm
    wamrlib
    ${WAMR_LLVM_LIBRARIES}
)
