# ----- Third-party Deps -----
include(ExternalProject)

set(SGX_EXTERNAL_DIR ${CMAKE_BINARY_DIR}/sgx-deps)
set(SGX_EXTERNAL_INSTALL_DIR ${SGX_EXTERNAL_DIR}/install)

# Define external project for GMP
ExternalProject_Add(gmp
    URL https://gmplib.org/download/gmp/gmp-6.2.1.tar.bz2
    PREFIX ${SGX_EXTERNAL_DIR}/gmp
    CONFIGURE_COMMAND ${SGX_EXTERNAL_DIR}/gmp/src/gmp/configure --prefix=${SGX_EXTERNAL_INSTALL_DIR} --enable-cxx > /dev/null
    BUILD_COMMAND make > /dev/null
    INSTALL_COMMAND make install > /dev/null
)

# Define external project for PBC
# TODO: stderr to dev/null too
ExternalProject_Add(pbc
    URL https://crypto.stanford.edu/pbc/files/pbc-0.5.14.tar.gz
    PREFIX ${SGX_EXTERNAL_DIR}/pbc
    CONFIGURE_COMMAND LDFLAGS=-L${SGX_EXTERNAL_INSTALL_DIR}/lib CPPFLAGS=-I${SGX_EXTERNAL_INSTALL_DIR}/include ./configure --prefix=${SGX_EXTERNAL_INSTALL_DIR} > /dev/null
    BUILD_IN_SOURCE 1
    BUILD_COMMAND make > /dev/null
    INSTALL_COMMAND make install > /dev/null
    DEPENDS gmp
)

set(FETCHCONTENT_QUIET ON)
FetchContent_Declare(relic_ext
    GIT_REPOSITORY "https://github.com/relic-toolkit/relic"
    GIT_TAG "32640e7410a0dbc812eb38689132f7340e8193d6"
)
FetchContent_MakeAvailable(relic_ext)
set(RELIC_INCLUDE_DIR "-I${relic_ext_SOURCE_DIR}/include -I${relic_ext_BINARY_DIR}/include")
message(STATUS "HELLO: ${RELIC_INCLUDE_DIR}")

# Define external project for OpenABE
ExternalProject_Add(openabe
    GIT_REPOSITORY https://github.com/zeutro/openabe.git
    PREFIX ${SGX_EXTERNAL_DIR}/openabe
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND bash -c ". ./env && make src INSTALL_PREFIX=${SGX_EXTERNAL_INSTALL_DIR} CXXFLAGS=\"$CXXFLAGS -I${SGX_EXTERNAL_DIR}/openabe/src/openabe/src/include -I${SGX_EXTERNAL_INSTALL_DIR}/include ${RELIC_INCLUDE_DIR}\""
    INSTALL_COMMAND make install
    DEPENDS pbc
)

# Set GMP, PBC, and OpenABE directories after build
ExternalProject_Get_Property(gmp INSTALL_DIR)
set(GMP_INCLUDE_DIR ${INSTALL_DIR}/include)
set(GMP_LIBRARIES ${INSTALL_DIR}/lib/libgmp.a)

ExternalProject_Get_Property(pbc INSTALL_DIR)
set(PBC_INCLUDE_DIR ${INSTALL_DIR}/include)
set(PBC_LIBRARIES ${INSTALL_DIR}/lib/libpbc.a)

ExternalProject_Get_Property(openabe INSTALL_DIR)
set(OPENABE_INCLUDE_DIR ${INSTALL_DIR}/include/openabe)
set(OPENABE_LIBRARIES ${INSTALL_DIR}/lib/libopenabe.a)

add_dependencies(openabe pbc gmp)

# ----- Trusted cyrpto libraries -----

set(ENCLAVE_TRUSTED_CRYPTO_HEADERS
    ${FAASM_INCLUDE_DIR}/enclave/inside/crypto/hash.h
    ${FAASM_INCLUDE_DIR}/enclave/inside/crypto/types.h
    ${FAASM_INCLUDE_DIR}/enclave/inside/crypto/encryption.h
)

set(ENCLAVE_TRUSTED_CRYPTO_SRC
    encryption.cpp
    hash.cpp
    ${FAASM_SGX_CRYPTO_HEADERS}
)

faasm_private_lib(enclave_trusted_crypto "${ENCLAVE_TRUSTED_CRYPTO_SRC}")

add_dependencies(enclave_trusted_crypto openabe)

target_compile_options(enclave_trusted_crypto PRIVATE
    ${ENCLAVE_TRUSTED_C_FLAGS}
)

target_include_directories(enclave_trusted_crypto PRIVATE
    ${SGX_SDK_PATH}/include
    ${SGX_SDK_PATH}/include/tlibc
    ${SGX_SDK_PATH}/include/libcxx
    ${GMP_INCLUDE_DIR}
    ${PBC_INCLUDE_DIR}
    ${OPENABE_INCLUDE_DIR}
)

target_link_libraries(enclave_trusted_crypto PUBLIC
    ${SGX_SDK_LIB_PATH}/libsgx_tstdc.a
    ${SGX_SDK_LIB_PATH}/libsgx_tcrypto.a
    ${GMP_LIBRARY}
    ${PBC_LIBRARY}
    ${OPENABE_LIBRARY}
)
