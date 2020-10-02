include(FindGit)
find_package(Git)
include (ExternalProject)
include (FetchContent)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

# Tightly-coupled dependencies
FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "faasm"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT= \
        -DWAVM_ENABLE_UNWIND=ON"
)

FetchContent_Declare(wamr_ext
    GIT_REPOSITORY "https://github.com/J-Heinemann/wasm-micro-runtime"
    GIT_TAG "5a3db31c096ec4d8a7b8b639aeaecd3fdda06b30"
)

FetchContent_Declare(faabric_ext
    GIT_REPOSITORY "https://github.com/faasm/faabric.git"
    GIT_TAG "v0.0.12"
    CMAKE_ARGS "-DFAABRIC_BUILD_TESTS=OFF"
)

FetchContent_MakeAvailable(wavm_ext faabric_ext wamr_ext)

# Allow access to headers nested in other projects
FetchContent_GetProperties(wavm_ext SOURCE_DIR FAASM_WAVM_SOURCE_DIR)
message(STATUS FAASM_WAVM_SOURCE_DIR ${FAASM_WAVM_SOURCE_DIR})

FetchContent_GetProperties(wamr_ext SOURCE_DIR WAMR_ROOT_DIR)
message(STATUS WAMR_ROOT_DIR ${WAMR_ROOT_DIR})

FetchContent_GetProperties(faabric_ext BINARY_DIR FAABRIC_BINARY_DIR)
message(STATUS FAABRIC_BINARY_DIR ${FAABRIC_BINARY_DIR})

# SGX-specific dependencies
if(FAASM_SGX_XRA)
    FetchContent_Declare(xra_ext
        GIT_REPOSITORY "https://github.com/J-Heinemann/eXtended-Remote-Attestation"
        GIT_TAG "1252f429c478d8c9052b02fc54f9f7d6ecc33594"
    )

    FetchContent_MakeAvailable(xra_ext)

    # Access to headers in XRA
    FetchContent_GetProperties(xra_ext SOURCE_DIR FAASM_XRA_ROOT_DIR)
    set(FAASM_XRA_INCLUDE_PATH ${FAASM_XRA_ROOT_DIR}/include)
endif()

# General 3rd party dependencies
ExternalProject_Add(eigen_ext
    GIT_REPOSITORY "https://gitlab.com/shillaker/eigen.git"
    GIT_TAG "faasm"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(cereal_ext
    GIT_REPOSITORY "https://github.com/USCiLab/cereal"
    GIT_TAG "v1.3.0"
    CMAKE_ARGS "-DJUST_INSTALL_CEREAL=1"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

ExternalProject_Add(catch2_ext
     GIT_REPOSITORY "https://github.com/catchorg/Catch2"
     GIT_TAG "v2.13.2"
     CMAKE_ARGS "-DCATCH_INSTALL_DOCS=OFF \
         -DCATCH_INSTALL_EXTRAS=OFF"
     CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)
 
