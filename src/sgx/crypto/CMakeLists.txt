set(ENCLAVE_TRUSTED_CRYPTO_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/crypto/hash.h
    ${FAASM_INCLUDE_DIR}/sgx/crypto/types.h
    ${FAASM_INCLUDE_DIR}/sgx/crypto/encryption.h
)

set(ENCLAVE_TRUSTED_CRYPTO_SRC
    encryption.cpp
    hash.cpp
    ${FAASM_SGX_CRYPTO_HEADERS}
)

faasm_private_lib(enclave_trusted_crypto "${ENCLAVE_TRUSTED_CRYPTO_SRC}")

target_compile_options(enclave_trusted_crypto PRIVATE
    ${ENCLAVE_TRUSTED_C_FLAGS}
)

target_include_directories(enclave_trusted_crypto PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
)

target_link_libraries(enclave_trusted_crypto PUBLIC
    ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
)
