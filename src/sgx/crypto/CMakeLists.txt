set(FAASM_SGX_CRYPTO_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/crypto/hash.h
    ${FAASM_INCLUDE_DIR}/sgx/crypto/types.h
    ${FAASM_INCLUDE_DIR}/sgx/crypto/encryption.h
    )

set(FAASM_SGX_CRYPTO_SRC
    encryption.cpp
    hash.cpp
    ${FAASM_SGX_CRYPTO_HEADERS}
    )

add_library(faasm_sgx_crypto STATIC ${FAASM_SGX_CRYPTO_SRC})

target_compile_options(faasm_sgx_crypto PRIVATE
    ${FAASM_SGX_ENCLAVE_C_FLAGS}
    ${FAASM_SGX_ENCLAVE_CXX_FLAGS}
    )

target_include_directories(faasm_sgx_crypto PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
    )

target_link_libraries(faasm_sgx_crypto
    -Wl,--start-group
    ${SGX_SDK_LIB_PATH}/libsgx_tcxx.a
    ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
    -Wl,--end-group
    )
