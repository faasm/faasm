include(FindGit)
find_package(Git)
include (ExternalProject)
include (FetchContent)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

# Tightly-coupled dependencies
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "faasm"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT="
)

FetchContent_Declare(wamr_ext
    GIT_REPOSITORY "https://github.com/J-Heinemann/wasm-micro-runtime"
    GIT_TAG "5a3db31c096ec4d8a7b8b639aeaecd3fdda06b30"
)

FetchContent_MakeAvailable(wavm_ext wamr_ext)

# Allow access to headers nested in other projects
FetchContent_GetProperties(wavm_ext SOURCE_DIR FAASM_WAVM_SOURCE_DIR)
message(STATUS FAASM_WAVM_SOURCE_DIR ${FAASM_WAVM_SOURCE_DIR})

FetchContent_GetProperties(wamr_ext SOURCE_DIR WAMR_ROOT_DIR)
message(STATUS WAMR_ROOT_DIR ${WAMR_ROOT_DIR})

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

# AWS docs on using as an external project:
# https://github.com/aws/aws-sdk-cpp/blob/main/Docs/CMake_External_Project.md
# If you need to build multiple AWS components, see this issue:
# https://github.com/aws/aws-sdk-cpp/issues/826
ExternalProject_Add(aws_ext
    GIT_REPOSITORY   https://github.com/aws/aws-sdk-cpp.git
    GIT_TAG          "b733384b16945818fa5da5b73e410dea1e9ab9d0"
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
    CMAKE_ARGS       -DBUILD_SHARED_LIBS=OFF
                     -DBUILD_ONLY=s3
                     -DAUTORUN_UNIT_TESTS=OFF
                     -DENABLE_TESTING=OFF
                     -DCMAKE_BUILD_TYPE=Release
    BUILD_ALWAYS     0
    TEST_COMMAND     ""
    UPDATE_COMMAND   ""
)

ExternalProject_Add(catch2_ext
     GIT_REPOSITORY "https://github.com/catchorg/Catch2"
     GIT_TAG "v2.13.2"
     CMAKE_ARGS "-DCATCH_INSTALL_DOCS=OFF \
         -DCATCH_INSTALL_EXTRAS=OFF"
     CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

