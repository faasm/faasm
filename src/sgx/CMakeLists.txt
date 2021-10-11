# --------------------------------------------------------
# General SGX Config and Checks
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

# NASM configuration
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS -felf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT
    "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>"
)

# --------------------------------------------------------
# SGX Compilation Flags
# --------------------------------------------------------

set(SGX_C_GLOBAL_FLAGS -m64)

if(CMAKE_BUILD_TYPE EQUAL "Debug")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O0 -g)

    set(ENCLAVE_UNTRUSTED_C_FLAGS
        ${SGX_C_GLOBAL_FLAGS}
        -fPIC
        -Wno-attributes
        -DDEBUG
        -UNDEBUG
        -UEDEBUG
    )

    add_definitions(-DFAASM_SGX_DEBUG)
else()
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O2)

    set(ENCLAVE_UNTRUSTED_C_FLAGS
        ${SGX_C_GLOBAL_FLAGS}
        -fPIC
        -Wno-attributes
    )
endif()

# Note - these are the same in debug/ non-debug mode
set(ENCLAVE_TRUSTED_C_FLAGS
    ${SGX_C_GLOBAL_FLAGS}
    -nostdinc
    -fvisibility=hidden
    -fpie
    -ffunction-sections
    -fdata-sections
    -fstack-protector-strong
)

set(ENCLAVE_TRSUTED_CXX_FLAGS
    -Wnon-virtual-dtor
    -std=c++11
    -nostdinc++
)

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
# Trusted Enclave Library
# --------------------------------------------------------

set(ENCLAVE_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/attestation.h
    ${FAASM_INCLUDE_DIR}/sgx/enclave_config.h
    ${FAASM_INCLUDE_DIR}/sgx/enclave_types.h
    ${FAASM_INCLUDE_DIR}/sgx/error.h
    ${FAASM_INCLUDE_DIR}/sgx/native.h
    ${FAASM_INCLUDE_DIR}/sgx/native_symbols_wrapper.h
    ${FAASM_INCLUDE_DIR}/sgx/rw_lock.h
    ${FAASM_INCLUDE_DIR}/sgx/SGXWAMRWasmModule.h
    ${FAASM_INCLUDE_DIR}/sgx/system.h
)

# SGX WAMR enclave library
set(ENCLAVE_TRUSTED_SRC
    checks.cpp
    enclave.cpp
    env.cpp
    filesystem.cpp
    funcs.cpp
    memory.cpp
    native.cpp
    pthread.cpp
    rw_lock.cpp
    state.cpp
    ${ENCLAVE_HEADERS}
)

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

target_compile_options(enclave_trusted PRIVATE
    ${ENCLAVE_TRUSTED_C_FLAGS}
    ${ENCLAVE_TRUSTED_CXX_FLAGS}
)

target_link_directories(enclave_trusted PRIVATE ${SGX_SDK_LIB_PATH})
target_link_options(enclave_trusted PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/${ENCLAVE_EDL_FILENAME}_t.o
    ${SGX_C_GLOBAL_FLAGS}
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles
    -Bstatic -Bsymbolic
    -Wl,-pie,-eenclave_entry
    -Wl,--export-dynamic
    -Wl,--defsym,__ImageBase=0
    -Wl,--gc-sections
    -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/enclave.lds
)

if(FAASM_SGX_SIM_MODE)
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts_sim.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice_sim.a)
else()
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice.a)
endif()

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
    attestation.cpp
    native_symbols.cpp
    rw_lock.cpp
    SGXWAMRWasmModule.cpp
    system.cpp
)

add_library(enclave_untrusted STATIC
    ${ENCLAVE_UNTRUSTED_HEADERS}
    ${ENCLAVE_UNTRUSTED_SRC}
    ${ENCLAVE_UNTRUSTED_ASM_SRC}
)

target_include_directories(enclave_untrusted PUBLIC ${SGX_SDK_PATH}/include)

target_compile_options(enclave_untrusted PRIVATE
    ${ENCLAVE_UNTRUSTED_C_FLAGS}
    -std=c++11
    -ffunction-sections
    -fdata-sections
)

add_dependencies(enclave_untrusted enclave_trusted)

target_link_directories(enclave_untrusted INTERFACE ${SGX_SDK_LIB_PATH})

target_link_options(enclave_untrusted PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${ENCLAVE_EDL_FILENAME}_u.o
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
    --trusted ${ENCLAVE_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/sgx
    --search-path ${SGX_SDK_PATH}/include
    --search-path ${WAMR_SHARED_DIR}/platform/linux-sgx/
)

add_custom_command(TARGET enclave_trusted
    PRE_BUILD COMMAND gcc
    ${ENCLAVE_TRUSTED_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -I${SGX_SDK_PATH}/include/tlibc
    -c ${ENCLAVE_EDL_FILENAME}_t.c
    -o ${ENCLAVE_EDL_FILENAME}_t.o
)

# TODO - sign with an actual key
add_custom_command(TARGET enclave_trusted
    POST_BUILD COMMAND
    ${SGX_SDK_ENCLAVE_SIGNER} sign
    -key ${CMAKE_CURRENT_SOURCE_DIR}/enclave.pem
    -enclave ${CMAKE_BINARY_DIR}/lib/enclave_trusted.so
    -out ${ENCLAVE_PATH}
    -config ${CMAKE_CURRENT_SOURCE_DIR}/enclave.config
)

# --------------------------------------------------------
# Unrusted Enclave Build + Signature
# --------------------------------------------------------

add_custom_command(TARGET enclave_untrusted
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --untrusted ${ENCLAVE_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/sgx
    --search-path ${SGX_SDK_PATH}/include
    --search-path ${WAMR_SHARED_DIR}/platform/linux-sgx/
)

add_custom_command(TARGET enclave_untrusted
    PRE_BUILD COMMAND gcc
    ${ENCLAVE_UNTRUSTED_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -c ${ENCLAVE_EDL_FILENAME}_u.c
    -o ${ENCLAVE_EDL_FILENAME}_u.o
)
