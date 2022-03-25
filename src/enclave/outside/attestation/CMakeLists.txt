add_library(attestation STATIC
    ${FAASM_INCLUDE_DIR}/enclave/outside/attestation/ecalls.h
    generateQuote.cpp
)

target_include_directories(attestation PUBLIC
    ${SGX_SDK_PATH}/include
    ${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)

# Link with ECalls
target_link_options(attestation PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${ATTESTATION_EDL_FILENAME}_u.o
    -Wl,--gc-sections
)

target_link_libraries(attestation
    faasm::common_deps
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    ${SGX_DCAP_LIB}
)

add_library(faasm::attestation ALIAS attestation)

# 24/03/2022 - If we link the attestation library with the main faasm::enclave
# target, we get a segmentation fault when trying to dlopen an SGX library. For
# the time being, we implement attestation as a separate binary that we call
# from the enclave.
add_executable(attest_enclave attest_enclave.cpp)
target_link_libraries(attest_enclave faasm::attestation)

add_dependencies(enclave_untrusted attest_enclave)
add_dependencies(attest_enclave wamrlib_untrusted)

# --------------------------------------------------------
# Build attestation ECall/OCall untrusted interface
# --------------------------------------------------------

add_custom_command(TARGET attestation
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --untrusted ${ATTESTATION_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/enclave/inside/attestation
    --search-path ${SGX_SDK_PATH}/include
)

add_custom_command(TARGET attestation
    PRE_BUILD COMMAND gcc
    ${ENCLAVE_UNTRUSTED_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -c ${ATTESTATION_EDL_FILENAME}_u.c
    -o ${ATTESTATION_EDL_FILENAME}_u.o
)

