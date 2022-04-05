# --------------------------------------------------------
# Intel SGX DCAP for remote attestation
# --------------------------------------------------------

set(SGX_DCAP_QUOTE_GENERATION "/opt/intel/sgxdcap/QuoteGeneration")
set(SGX_DCAP_QUOTE_GENERATION_INCLUDE "${SGX_DCAP_QUOTE_GENERATION}/quote_wrapper")
set(SGX_DCAP_LIB /usr/lib/libsgx_dcap_ql.so)

add_library(attestation STATIC
    attestation.cpp
    generateQuote.cpp
    validateQuote.cpp
    AzureAttestationServiceClient.cpp
    EnclaveInfo.cpp
)

target_include_directories(attestation PUBLIC
    ${SGX_SDK_PATH}/include
    ${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)

# We need to link with cpprest to interact with the remote attestation service
# over HTTP. We use that the library is installed through faabric.
find_package(cpprestsdk REQUIRED)
find_package(cppcodec REQUIRED)

target_link_libraries(attestation
    faasm::common_deps
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    ${SGX_DCAP_LIB}
    cppcodec::cppcodec
    cpprestsdk::cpprestsdk
    jwt-cpp::jwt-cpp
    picojson::picojson
)

add_library(faasm::attestation ALIAS attestation)
