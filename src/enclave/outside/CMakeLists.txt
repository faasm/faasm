# --------------------------------------------------------
# Untrusted WAMR library
# --------------------------------------------------------

add_library(wamrlib_untrusted STATIC ${PLATFORM_SHARED_SOURCE_UNTRUSTED})

target_compile_options(wamrlib_untrusted PRIVATE
    -fPIC
    -ffunction-sections
    -fdata-sections
    -Wno-sign-compare
    -Wno-unused-variable
)

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
    ${FAASM_INCLUDE_DIR}/enclave/outside/ecalls.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/getSgxSupport.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/system.h
    ${FAASM_INCLUDE_DIR}/enclave/outside/EnclaveInterface.h
)

set(ENCLAVE_UNTRUSTED_SRC
    getSgxSupport.cpp
    ocalls.cpp
    system.cpp
    EnclaveInterface.cpp
)

faasm_private_lib(enclave_untrusted
    ${ENCLAVE_UNTRUSTED_HEADERS}
    ${ENCLAVE_UNTRUSTED_SRC}
)

target_include_directories(enclave_untrusted PUBLIC ${SGX_SDK_PATH}/include)

target_compile_options(enclave_untrusted PRIVATE
    ${ENCLAVE_UNTRUSTED_C_FLAGS}
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
    set(SGX_UNTRUSTED_RUNTIME_LIB sgx_urts)
    set(SGX_UAE_SERVICE_LIB sgx_uae_service)
    set(SGX_CAPABLE_LIB sgx_capable)
else()
    message(FATAL_ERROR "Building SGX code with mode: ${FAASM_SGX_MODE}")
endif()

# --------------------------------------------------------
# Remote attestation
# --------------------------------------------------------

if (FAASM_SGX_MODE STREQUAL "Hardware" OR FAASM_SGX_MODE STREQUAL "Simulation")
    add_subdirectory(attestation)
else ()
    add_library(attestation_dummy INTERFACE)
    add_library(faasm::attestation ALIAS attestation_dummy)
endif ()

# --------------------------------------------------------
# Link everything together and add alias
# --------------------------------------------------------

target_link_libraries(enclave_untrusted PUBLIC
    faasm::common_deps
    ${SGX_UNTRUSTED_RUNTIME_LIB}
    ${SGX_UAE_SERVICE_LIB}
    ${SGX_CAPABLE_LIB}
    faasm::attestation
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
    -I${WAMR_ROOT_DIR}/core/iwasm/include
    -c ${ENCLAVE_EDL_FILENAME}_u.c
    -o ${ENCLAVE_EDL_FILENAME}_u.o
)

# --------------------------------------------------------
# Helper binary to detect SGX from the command line
# --------------------------------------------------------

add_executable(detect_sgx detect_sgx.cpp)
target_link_libraries(detect_sgx faasm::enclave)
