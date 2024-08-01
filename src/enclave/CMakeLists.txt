# --------------------------------------------------------
# SGX Config and Checks Shared Between Trusted/Untrusted Builds
# --------------------------------------------------------

# SGX configuration
set(SGX_SDK_LIB_PATH ${SGX_SDK_PATH}/lib64)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)
set(ENCLAVE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enclave_trusted.sign.so")
set(ENCLAVE_EDL_FILENAME "enclave")
add_definitions(-DFAASM_ENCLAVE_PATH="${ENCLAVE_PATH}")

# Check for SGX SDK
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(EXISTS ${SGX_SDK_PATH})
    message(STATUS "Found SGX-SDK: TRUE")
else()
    message(STATUS "Found SGX-SDK: FALSE")
    message(FATAL_ERROR "SGX-SDK not installed in ${SGX_SDK_PATH}")
endif()

# --------------------------------------------------------
# Global SGX Compilation Flags
# --------------------------------------------------------

set(SGX_C_GLOBAL_FLAGS -m64)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O0 -g)

    # add_definitions(-DFAASM_SGX_DEBUG)
    add_compile_definitions(FAASM_SGX_DEBUG)
else()
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O2)
endif()


# --------------------------------------------------------
# WAMR Build Common For Trusted and Untrusted
#
# 28/06/2021 - To build WAMR inside SGX, we follow the provided example:
# https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/product-mini/platforms/linux-sgx/CMakeLists.txt
# --------------------------------------------------------

# Set target platform details
set(WAMR_BUILD_PLATFORM "linux-sgx")
set(WAMR_BUILD_TARGET X86_64)
set(WAMR_BUILD_SPEC_TEST)

# Set AOT mode, disable JIT
set(WAMR_BUILD_AOT 1)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_LAZY_JIT 0)

# Set libraries
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_LIB_PTHREAD 0)

# Let WAMR do the including and importing of the sources
include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

# --------------------------------------------------------
# Trusted Enclave Library
# --------------------------------------------------------
add_subdirectory(inside)

# --------------------------------------------------------
# Untrusted Enclave Library
# --------------------------------------------------------
add_subdirectory(outside)

# Set dependency so that trusted code gets built with all major CMake targets
add_dependencies(enclave_untrusted enclave_trusted)
