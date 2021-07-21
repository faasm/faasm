set(FAASM_SGX_CRYPTO_HEADERS
    ${FAASM_INCLUDE_DIR}/sgx/crypto/utils.h
    )

set(FAASM_SGX_CRYPTO_SRC
    utils.cpp
    ${FAASM_SGX_CRYPTO_HEADERS}
    )

add_library(faasm_sgx_crypto STATIC ${FAASM_SGX_CRYPTO_SRC})

target_compile_options(faasm_sgx_crypto PRIVATE
    -nostdinc
    -fvisibility=hidden
    -fpie
    -ffunction-sections
    -fdata-sections
    -fstack-protector-strong
    )

target_include_directories(faasm_sgx_crypto PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
    )

target_link_libraries(faasm_sgx_crypto
    ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
    )
