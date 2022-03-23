# --------------------------------------------------------
# Untrusted SGX Compilation Flags
# --------------------------------------------------------

if(CMAKE_BUILD_TYPE EQUAL "Debug")
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
    set(ENCLAVE_UNTRUSTED_C_FLAGS
        ${SGX_C_GLOBAL_FLAGS}
        -fPIC
        -Wno-attributes
    )
endif()

# --------------------------------------------------------
# Untrusted Enclave Library
# --------------------------------------------------------

set(ENCLAVE_UNTRUSTED_HEADERS
    ${FAASM_INCLUDE_DIR}/enclave/error.h
    # ${FAASM_INCLUDE_DIR}/enclave/outside/attestation.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/ecalls.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/getSgxSupport.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/system.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/EnclaveInterface.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/EnclaveQuote.h
)

set(ENCLAVE_UNTRUSTED_SRC
	# attestation.cpp
    getSgxSupport.cpp
    ocalls.cpp
    system.cpp
    EnclaveInterface.cpp
    EnclaveQuote.cpp
)

add_library(enclave_untrusted STATIC
    ${ENCLAVE_UNTRUSTED_HEADERS}
    ${ENCLAVE_UNTRUSTED_SRC}
)

target_include_directories(enclave_untrusted PUBLIC ${SGX_SDK_PATH}/include)

target_compile_options(enclave_untrusted PRIVATE
    ${ENCLAVE_UNTRUSTED_C_FLAGS}
    -std=c++11
    -ffunction-sections
    -fdata-sections
)

target_link_options(enclave_untrusted PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${ENCLAVE_EDL_FILENAME}_u.o
    -Wl,--gc-sections
)

# --------------------------------------------------------
# Intel SGX SDK and PSW
# --------------------------------------------------------

# It is important to link to the SDK lib path _only_ in simulation mode. In
# HW mode we must link to the system libraries. Note that this only applies to
# code living outside the enclave.
if (FAASM_SGX_MODE STREQUAL "Simulation")
    target_link_directories(enclave_untrusted INTERFACE ${SGX_SDK_LIB_PATH})
    set(SGX_UNTRUSTED_RUNTIME_LIB ${SGX_SDK_LIB_PATH}/libsgx_urts_sim.so)
    set(SGX_UAE_SERVICE_LIB ${SGX_SDK_LIB_PATH}/libsgx_uae_service_sim.so)
    set(SGX_CAPABLE_LIB ${SGX_SDK_LIB_PATH}/libsgx_capable.so)
elseif (FAASM_SGX_MODE STREQUAL "Hardware")
    set(SGX_UNTRUSTED_RUNTIME_LIB /usr/lib/libsgx_urts.so)
    set(SGX_UAE_SERVICE_LIB /usr/lib/libsgx_uae_service.so)
    set(SGX_CAPABLE_LIB /usr/lib/libsgx_capable.so)
else()
    message(FATAL_ERROR "Building SGX code with mode: ${FAASM_SGX_MODE}")
endif()

# --------------------------------------------------------
# Intel DCAP for remote attestation
# --------------------------------------------------------

if (FAASM_SGX_MODE STREQUAL "Simulation")
    set(SGX_DCAP_LIBS
        ${SGX_DCAP_QUOTE_GENERATION_LIB_PATH}/libsgx_dcap_ql.so
    )
elseif (FAASM_SGX_MODE STREQUAL "Hardware")
    set(SGX_DCAP_LIB /usr/lib/libsgx_dcap_ql.so)
else ()
    message(FATAL_ERROR "DCAP not ready for mode: ${FAASM_SGX_MODE}")
endif()

target_include_directories(enclave_untrusted PUBLIC
    ${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)

add_subdirectory(attestation)

# --------------------------------------------------------
# Link everything together and add alias
# --------------------------------------------------------

target_link_libraries(enclave_untrusted
    faasm::common_deps
    ${SGX_CAPABLE_LIB}
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    attestation
    # ${SGX_DCAP_LIB}
    # ${SGX_UAE_SERVICE_LIB}
    Threads::Threads
    wamrlib_untrusted
    wasm
)

add_library(faasm::enclave ALIAS enclave_untrusted)

# --------------------------------------------------------
# Unrusted Enclave Build
# --------------------------------------------------------

add_custom_command(TARGET enclave_untrusted
    PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
    --untrusted ${ENCLAVE_EDL_FILENAME}.edl
    --search-path ${FAASM_SOURCE_DIR}/enclave/inside
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

# --------------------------------------------------------
# Helper binary to detect SGX from the command line
# --------------------------------------------------------

add_executable(detect_sgx detect_sgx.cpp)
target_link_libraries(detect_sgx faasm::enclave)

# --------------------------------------------------------
# Test to see what is going on
# --------------------------------------------------------

# This works
add_executable(test_dlopen test.cpp)
target_include_directories(test_dlopen PUBLIC
    	${SGX_SDK_PATH}/include
    	${SGX_DCAP_QUOTE_GENERATION_INCLUDE}/ql/inc
)
target_link_libraries(test_dlopen
	faasm::common_deps
	${SGX_CAPABLE_LIB}
	${SGX_UNTRUSTED_RUNTIME_LIB}
	${SGX_DCAP_LIB}
	# ${SGX_UAE_SERVICE_LIB}
	Threads::Threads
)

# This also works
# Note: worst case scenario we just build a binary to do the attestation 
add_executable(attestation_exe test2.cpp)
target_link_libraries(attestation_exe attestation)
