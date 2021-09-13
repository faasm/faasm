# Unfortunately the pre-canned WAMR CMake file modifies too many top-level
# variables (e.g. through use of include_directories or changing global
# CFLAGS).
#
# We have our own custom CMake set-up which looks a bit like the wamrc build:
# https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/wamr-compiler/CMakeLists.txt

# Set target platform details
set (WAMR_BUILD_PLATFORM "linux")
set (WAMR_BUILD_TARGET X86_64)
set (WAMR_BUILD_SPEC_TEST 0)

# # Set AOT mode and JIT for code generation
set (WAMR_BUILD_AOT 1)
set (WAMR_BUILD_JIT 1)
set (WAMR_BUILD_LAZY_JIT 0)

# Set libraries
set (WAMR_BUILD_LIBC_BUILTIN 0)
set (WAMR_BUILD_LIBC_WASI 1)
set (WAMR_BUILD_LIB_PTHREAD 0)

set (WAMR_BUILD_MULTI_MODULE 1)
# Explicitely disable these two options, as they cause runtime errors
set (WAMR_BUILD_BULK_MEMORY 0)
set (WAMR_DISABLE_HW_BOUND_CHECK 0)
# TODO: why?
# Somehow this definition prevents a buffer underflow
add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)

# TODO: At this point we will need to use the system LLVM first
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
# Enabling JIT-ing introduces a warning with libunwind and a segfault when
# generating code. Setting the CMake flag includes the code required to run
# code generation, but the pre-processor macro introduces LLVM errors.
remove_definitions("-DWASM_ENABLE_JIT=1")
faasm_private_lib(wamrlib "${WAMR_RUNTIME_LIB_SOURCE}")

# -----------------------------
# Faasm-specific configuration
# -----------------------------

set(PLATFORM_HEADER ${WAMR_SHARED_DIR}/platform/linux)

target_include_directories(
    wamrlib PUBLIC
    ${FAASM_INCLUDE_DIR}/wamr
    ${PLATFORM_HEADER}
    )

set(HEADERS
    "${FAASM_INCLUDE_DIR}/wamr/native.h"
    "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
)

set(LIB_FILES
        WAMRWasmModule.cpp
        codegen.cpp
        dynlink.cpp
        filesystem.cpp
        funcs.cpp
        memory.cpp
        native.cpp
        pthread.cpp
        state.cpp
        stubs.cpp
        ${HEADERS}
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
faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule
    wasm
    wamrlib
    ${WAMR_LLVM_LIBRARIES}
)
