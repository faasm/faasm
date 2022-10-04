# WARNING: the CMake flags and pre-processor definitions that we setin this
# file are carefully chosen for the WAMR build to run. Adding or removing flags
# may break Faasm.

# Set target platform details
set(WAMR_BUILD_PLATFORM "linux")
set(WAMR_BUILD_TARGET X86_64)
set(WAMR_BUILD_SPEC_TEST 0)

# Top-level Faasm flag for WAMR
add_definitions(-DWAMR_FAASM=1)

# Set AOT mode and JIT for code generation
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 1)
set(WAMR_BUILD_LAZY_JIT 0)

# Set libraries
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_LIB_PTHREAD 0)

# WAMR features
set(WAMR_BUILD_SIMD 0)

set(WAMR_BUILD_MULTI_MODULE 1)
# If bulk memory set, WAMR allocates one single page, breaking our brk memory
# management model
set(WAMR_BUILD_BULK_MEMORY 0)

# We must enable WAMR hardware bounds check here, otherwise WAMR uses malloc to
# allocate memory, which is not page-aligned. This seems like a blunt instrument
# to solve a smal problem, but this parameter is deeply embedded in the WAMR
# code, and it's difficult to change it surgically.
set(WAMR_DISABLE_HW_BOUND_CHECK 0)
# This definition prevents a buffer underflow during code generation
add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)

include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
# 13/09/2021 - Setting the CMake flag includes the code required to run
# code generation, but the pre-processor macro introduces LLVM errors. It is
# safe to disable as we actually don't run JITed code.
remove_definitions("-DWASM_ENABLE_JIT=1")
faasm_private_lib(wamrlib "${WAMR_RUNTIME_LIB_SOURCE}")

# Disable WAMR warnings
target_compile_options(wamrlib PRIVATE
    -Wno-typedef-redefinition
    -Wno-unused-command-line-argument
    # We comment out some problematic LLVM code in WAMR. Commenting out triggers
    # compilation warnings that we suppress
    -Wno-ambiguous-reversed-operator
    -Wno-unused-function
    -Wno-deprecated-enum-enum-conversion
)

# -----------------------------
# Faasm-specific configuration
# -----------------------------

target_include_directories(wamrlib PUBLIC
    ${PLATFORM_SHARED_DIR}
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
faasm_private_lib(wamrmodule
    WAMRWasmModule.cpp
    chaining.cpp
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
)
target_link_libraries(wamrmodule PUBLIC
    faasm::wasm
    faasm::wamrlib
    ${WAMR_LLVM_LIBRARIES}
)
