# --------------------------------------------------------
#                        WARNING
#
# Before modifying please read /docs/sgx.md
# This file contains many low level settings
# --------------------------------------------------------

# NASM configuration
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS -felf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>")

# FAASM-SGX enclave configuration
set(FAASM_SGX_ENCLAVE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/enclave_trusted.sign.so")

# Enable(1)/ Disable(0) SGX-specific debug/error messages
# Disable for better performance
set(FAASM_SGX_DEBUG 1)

# SGX configuration
set(SGX_DEBUG_MODE 1)
set(SGX_SDK_LIB_PATH ${SGX_SDK_PATH}/lib64)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)

set(SGX_C_GLOBAL_FLAGS -m64)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)

# --------------------------------------------------------
# WAMR Build
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

# WAMR Trusted lib
faasm_private_lib(wamrlib_trusted "${WAMR_RUNTIME_LIB_SOURCE}")

target_include_directories(wamrlib_trusted PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
)

target_compile_options(wamrlib_trusted PRIVATE
    -std=gnu99
    -fPIC
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wno-unused-parameter
    -Wno-pedantic
    -nostdinc
    -fvisibility=hidden
)

# WAMR untrusted lib
add_library(wamrlib_untrusted STATIC ${PLATFORM_SHARED_SOURCE_UNTRUSTED})

target_compile_options(wamrlib_untrusted PRIVATE
    -fPIC
    -ffunction-sections
    -fdata-sections
)

# --------------------------------------------------------
# Attestation configuration
# --------------------------------------------------------

# IP-address or hostname is possible to specify host
add_definitions(
    -DFAASM_SGX_ATTESTATION_HOST="localhost"
    -DFAASM_SGX_ATTESTATION_PORT=8009
    -DFAASM_SGX_ATTESTATION_CALLBACK_STORE_INIT_LEN=4
)

# eXtended Remote Attestation XRA configuration
if(FAASM_SGX_XRA)
    add_definitions(-DFAASM_SGX_XRA=1)
endif()

# Create definitions if necessary
add_definitions(
    -DFAASM_SGX_DEBUG=${FAASM_SGX_DEBUG}
    -DFAASM_SGX_ENCLAVE_PATH="${FAASM_SGX_ENCLAVE_PATH}"
)

# Debug message
message("-- Faasm-SGX Configuration:
 FAASM-SGX Debug Mode: ${FAASM_SGX_DEBUG}
 FAASM-SGX SGX Simulation Mode: ${FAASM_SGX_SIM_MODE}
 FAASM-SGX ATTESTATION: ${FAASM_SGX_ATTESTATION}
 FAASM-SGX XRA: ${FAASM_SGX_XRA}
 FAASM_SGX Enclave Path: ${FAASM_SGX_ENCLAVE_PATH}"
)

# --------------------------------------------------------
# Initial Checks
# --------------------------------------------------------

# Check for SGX SDK
find_package(Threads REQUIRED)
if(EXISTS ${SGX_SDK_PATH})
    message(STATUS "Found SGX-SDK: TRUE")
else()
    message(STATUS "Found SGX-SDK: FALSE")
    message(FATAL_ERROR "SGX-SDK not installed in ${SGX_SDK_PATH}")
endif()

# SGX compilation flags
if(SGX_DEBUG_MODE)
    message(STATUS "Enclave will be built with debug information and -O0 optimization")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O0 -g)

    set(FAASM_SGX_C_FLAGS ${SGX_C_GLOBAL_FLAGS}
        -fPIC
        -Wno-attributes
        -DDEBUG
        -UNDEBUG
        -UEDEBUG
    )
else()
    message(STATUS "Enclave will be build without debug information and -O2 optimization")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O2)

    set(FAASM_SGX_C_FLAGS ${SGX_C_GLOBAL_FLAGS}
        -fPIC
        -Wno-attributes
    )
endif()

# --------------------------------------------------------
# Trusted Enclave Library
# --------------------------------------------------------

set(ENCLAVE_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/enclaveConfig.h
    ${FAASM_INCLUDE_DIR}/sgx/attestation.h
    ${FAASM_INCLUDE_DIR}/sgx/enclave_types.h
    ${FAASM_INCLUDE_DIR}/sgx/error.h
    ${FAASM_INCLUDE_DIR}/sgx/native_symbols_wrapper.h
    ${FAASM_INCLUDE_DIR}/sgx/system.h
    ${FAASM_INCLUDE_DIR}/sgx/rw_lock.h
    ${FAASM_INCLUDE_DIR}/sgx/SGXWAMRWasmModule.h
)

# SGX WAMR enclave library
set(ENCLAVE_TRUSTED_SRC
    checks.cpp
    enclave.cpp
    native_symbols_wrapper.cpp
    rw_lock.cpp
    ${ENCLAVE_HEADERS}
)

# TODO - could we move to faasm_private_lib ?
add_library(enclave_trusted SHARED "${ENCLAVE_TRUSTED_SRC}")

target_include_directories(enclave_trusted PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
    ${WAMR_ROOT_DIR}/core
    ${WAMR_ROOT_DIR}/core/shared/utils
    ${WAMR_ROOT_DIR}/core/shared/platform/linux-sgx
)

set_target_properties(enclave_trusted PROPERTIES PREFIX "")

# ---------- Compilation flags ----------
# Note - these are the same in debug/ non-debug mode
set(ENCLAVE_C_FLAGS
    ${SGX_C_GLOBAL_FLAGS}
    -nostdinc
    -fvisibility=hidden
    -fpie
    -ffunction-sections
    -fdata-sections
    -fstack-protector-strong
)

set(ENCLAVE_CXX_FLAGS
    -Wnon-virtual-dtor
    -std=c++11
    -nostdinc++
)

target_compile_options(enclave_trusted PRIVATE
    ${ENCLAVE_C_FLAGS}
    ${ENCLAVE_CXX_FLAGS}
)

target_link_directories(enclave_trusted PRIVATE ${SGX_SDK_LIB_PATH})
target_link_options(enclave_trusted PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/enclave_t.o
    ${SGX_C_GLOBAL_FLAGS}
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles
    -Bstatic -Bsymbolic
    -Wl,-pie,-eenclave_entry
    -Wl,--export-dynamic
    -Wl,--defsym,__ImageBase=0
    -Wl,--gc-sections
    -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/enclave.lds
)

if (FAASM_SGX_SIM_MODE)
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts_sim.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice_sim.a)
else ()
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice.a)
endif ()

# Enclave trusted crypto
add_subdirectory(crypto)


# Common libraries
target_link_libraries(enclave_trusted
    -Wl,--whole-archive
    ${SGX_TRUSTED_RUNTIME_LIB}
    -Wl,--no-whole-archive
    -Wl,--start-group
    ${SGX_SDK_LIB_PATH}/libsgx_pthread.a
    ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcxx.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
    ${SGX_SERVICE_LIB}
    enclave_trusted_crypto
    wamrlib_trusted
    -Wl,--end-group
)

# --------------------------------------------------------
# Untrusted Enclave Library
# --------------------------------------------------------

set(ENCLAVE_UNTRUSTED_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/error.h
    ${FAASM_INCLUDE_DIR}/sgx/system.h
    ${FAASM_INCLUDE_DIR}/sgx/SGXWAMRWasmModule.h
)

set(ENCLAVE_UNTRUSTED_ASM_SRC
    SGXWAMRWasmModule.S
)

set_source_files_properties(${ENCLAVE_UNTRUSTED_ASM_SRC}
    PROPERTIES LANGUAGE ASM_NASM
)

set(ENCLAVE_UNTRUSTED_SRC
    SGXWAMRWasmModule.cpp
    system.cpp
    native_symbols.cpp
)

if(ENCLAVE_UNTRUSTED_SRC)
    set(ENCLAVE_UNTRUSTED_SRC ${ENCLAVE_UNTRUSTED_SRC}
            attestation.cpp
            rw_lock.cpp
    )
endif()


# TODO - can we use faasm_private_lib ?
add_library(enclave_untrusted STATIC
    ${ENCLAVE_UNTRUSTED_HEADERS}
    ${ENCLAVE_UNTRUSTED_SRC}
    ${ENCLAVE_UNTRUSTED_ASM_SRC}
)

target_include_directories(enclave_untrusted PUBLIC ${SGX_SDK_PATH}/include)

target_compile_options(enclave_untrusted PRIVATE
    ${FAASM_SGX_C_FLAGS}
    -std=c++11
    -ffunction-sections
    -fdata-sections
)

add_dependencies(enclave_untrusted enclave_trusted)

target_link_directories(enclave_untrusted INTERFACE ${SGX_SDK_LIB_PATH})

target_link_options(enclave_untrusted PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/enclave_u.o
    -Wl,--gc-sections
)

if(FAASM_SGX_SIM_MODE)
    set(SGX_UNTRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_urts_sim.so)
    set(SGX_UAE_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_uae_service_sim.so)
else()
    set(SGX_UNTRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_urts.so)
    set(SGX_UAE_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_uae_service.so)
endif()

target_link_libraries(enclave_untrusted
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    ${SGX_UAE_SERVICE_LIB}
    Threads::Threads
    wamrlib_untrusted
    wasm
)

# --------------------------------------------------------
# Trusted Enclave Build + Signature
# --------------------------------------------------------

add_custom_command(TARGET enclave_trusted
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --trusted enclave.edl
    --search-path ${FAASM_SOURCE_DIR}/sgx
    --search-path ${SGX_SDK_PATH}/include
    --search-path ${WAMR_SHARED_DIR}/platform/linux-sgx/
)

add_custom_command(TARGET enclave_trusted
    PRE_BUILD COMMAND gcc
    ${ENCLAVE_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -I${SGX_SDK_PATH}/include/tlibc
    -c enclave_t.c
    -o enclave_t.o
)

# TODO - sign with an actual key
add_custom_command(TARGET enclave_trusted
    POST_BUILD COMMAND
    ${SGX_SDK_ENCLAVE_SIGNER} sign
    -key ${CMAKE_CURRENT_SOURCE_DIR}/enclave.pem
    -enclave ${CMAKE_BINARY_DIR}/lib/enclave_trusted.so
    -out ${CMAKE_BINARY_DIR}/bin/enclave_trusted.sign.so
    -config ${CMAKE_CURRENT_SOURCE_DIR}/enclave.config
)

# --------------------------------------------------------
# Unrusted Enclave Build + Signature
# --------------------------------------------------------

add_custom_command(TARGET enclave_untrusted
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --untrusted enclave.edl
    --search-path ${FAASM_SOURCE_DIR}/sgx
    --search-path ${SGX_SDK_PATH}/include
    --search-path ${WAMR_SHARED_DIR}/platform/linux-sgx/
)

add_custom_command(TARGET enclave_untrusted
    PRE_BUILD COMMAND gcc
    ${FAASM_SGX_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -c enclave_u.c
    -o enclave_u.o
)
