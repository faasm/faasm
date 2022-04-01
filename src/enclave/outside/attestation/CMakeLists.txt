# --------------------------------------------------------
# Intel SGX DCAP for remote attestation
# --------------------------------------------------------

set(SGX_DCAP_QUOTE_GENERATION "/opt/intel/sgxdcap/QuoteGeneration")
set(SGX_DCAP_QUOTE_GENERATION_INCLUDE "${SGX_DCAP_QUOTE_GENERATION}/quote_wrapper")
set(SGX_DCAP_LIB /usr/lib/libsgx_dcap_ql.so)

add_library(attestation STATIC
    EnclaveInfo.cpp
    generateQuote.cpp
    validateQuote.cpp
)

target_include_directories(attestation PUBLIC
    ${SGX_SDK_PATH}/include
    ${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)

target_link_libraries(attestation
    faasm::common_deps
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    ${SGX_DCAP_LIB}
)

add_library(faasm::attestation ALIAS attestation)
