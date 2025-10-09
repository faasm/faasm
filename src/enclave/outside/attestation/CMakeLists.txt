# --------------------------------------------------------
# Intel SGX DCAP for remote attestation
# --------------------------------------------------------

set(SGX_DCAP_QUOTE_GENERATION "/opt/intel/sgxdcap/QuoteGeneration")
set(SGX_DCAP_QUOTE_GENERATION_INCLUDE "${SGX_DCAP_QUOTE_GENERATION}/quote_wrapper")
set(SGX_DCAP_LIB sgx_dcap_ql)

add_library(attestation STATIC
    attestation.cpp
    AttestationServiceClient.cpp
    EnclaveInfo.cpp
)

target_include_directories(attestation PUBLIC
    ${SGX_SDK_PATH}/include
    ${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)

find_package(cppcodec REQUIRED)

target_link_libraries(attestation
    faasm::common_deps
    cppcodec::cppcodec
    jwt-cpp::jwt-cpp
    picojson::picojson
    rapidjson
)

# Quote generation is only supported in hardware mode. Therefore, we only link
# against SGX libraries in hardware mode. Quote validation needs no SGX features
# and we want to also test it in simulation mode.
if (FAASM_SGX_MODE STREQUAL "Hardware")
    target_link_libraries(attestation
        ${SGX_UNTRUSTED_RUNTIME_LIB}
        ${SGX_DCAP_LIB}
    )
endif ()

add_library(faasm::attestation ALIAS attestation)
