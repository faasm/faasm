# TODO - after this works, remove duplicated flags
add_library(enclave_trusted_attestation SHARED ecalls.cpp)

target_include_directories(enclave_trusted_attestation PRIVATE
    ${FAASM_INCLUDE_DIR}
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
)

target_compile_options(enclave_trusted_attestation PRIVATE
    ${ENCLAVE_TRUSTED_C_FLAGS}
    ${ENCLAVE_TRUSTED_CXX_FLAGS}
)

target_link_directories(enclave_trusted_attestation PRIVATE ${SGX_SDK_LIB_PATH})
target_link_options(enclave_trusted_attestation PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/${ATTESTATION_EDL_FILENAME}_t.o
    ${SGX_C_GLOBAL_FLAGS}
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles
    -Bstatic -Bsymbolic
    -Wl,-pie,-eenclave_entry
    -Wl,--export-dynamic
    -Wl,--defsym,__ImageBase=0
    -Wl,--gc-sections
)

# target_link_libraries(enclave_trusted_attestation
#     -Wl,--whole-archive
#     ${SGX_TRUSTED_RUNTIME_LIB}
#     -Wl,--no-whole-archive
#     -Wl,--start-group
#     ${SGX_SDK_LIB_PATH}/libsgx_pthread.a
#     ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
#     ${SGX_SDK_LIB_PATH}/libsgx_tcxx.a
#     ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
#     ${SGX_SERVICE_LIB}
#     -Wl,--end-group
# )

# --------------------------------------------------------
# Build attestation ECall/OCall trusted interface
# --------------------------------------------------------

add_custom_command(TARGET enclave_trusted_attestation
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --trusted ${ATTESTATION_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/enclave/inside/attestation
    --search-path ${SGX_SDK_PATH}/include
)

add_custom_command(TARGET enclave_trusted_attestation
    PRE_BUILD COMMAND gcc
    ${ENCLAVE_TRUSTED_C_FLAGS}
    -I${SGX_SDK_PATH}/include
    -I${SGX_SDK_PATH}/include/tlibc
    -c ${ATTESTATION_EDL_FILENAME}_t.c
    -o ${ATTESTATION_EDL_FILENAME}_t.o
)
