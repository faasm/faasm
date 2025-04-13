# --------------------------------------------------------
# WAMR Trusted lib
# --------------------------------------------------------

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
    # These are needed to supress warnings when building WAMR
    -Wno-macro-redefined
    -Wno-pointer-sign
    -Wno-unused-command-line-argument
    -Wno-unused-parameter
    -Wno-typedef-redefinition
    -Wno-pedantic
    -nostdinc
    -fvisibility=hidden
)

# --------------------------------------------------------
# Trusted SGX Compilation Flags
# --------------------------------------------------------

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

set(ENCLAVE_TRUSTED_CXX_FLAGS
    -frtti
    -Wnon-virtual-dtor
    # https://github.com/intel/linux-sgx/issues/545
    -Wno-implicit-exception-spec-mismatch
    -Wno-typedef-redefinition
    -std=c++11
    -nostdinc++
)

# --------------------------------------------------------
# Trusted Enclave Library
# --------------------------------------------------------

set(ENCLAVE_TRUSTED_HEADERS
    ${FAASM_INCLUDE_DIR}/enclave/error.h
    ${FAASM_INCLUDE_DIR}/enclave/inside/EnclaveWasmModule.h
    ${FAASM_INCLUDE_DIR}/enclave/inside/native.h
    ${FAASM_INCLUDE_DIR}/enclave/inside/ocalls.h
)

set(ENCLAVE_TRUSTED_SRC
    EnclaveWasmModule.cpp
    attestation.cpp
    checks.cpp
    ecalls.cpp
    env.cpp
    filesystem.cpp
    funcs.cpp
    memory.cpp
    native.cpp
    pthread.cpp
    s3.cpp
    tests.cpp
    ${ENCLAVE_TRUSTED_HEADERS}
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

if (FAASM_SGX_MODE STREQUAL "Simulation")
    set(SGX_ENCLAVE_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/enclave_sim.config)
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts_sim.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice_sim.a)
elseif (FAASM_SGX_MODE STREQUAL "Hardware")
    set(SGX_ENCLAVE_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/enclave_hw.config)
    set(SGX_TRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_trts.a)
    set(SGX_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_tservice.a)
else ()
    message(FATAL_ERROR "Building SGX code with mode: ${FAASM_SGX_MODE}")
endif ()

# Enclave trusted crypto
add_subdirectory(crypto)

# Common libraries
target_link_libraries(enclave_trusted
    faasm::common_deps
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
# Trusted Enclave Build + Signature
# --------------------------------------------------------

add_custom_command(TARGET enclave_trusted
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --trusted ${ENCLAVE_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/enclave/inside
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
    -config ${SGX_ENCLAVE_CONFIG_FILE}
)

