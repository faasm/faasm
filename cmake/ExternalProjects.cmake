include(FindGit)
find_package(Git)
include (ExternalProject)
include (FetchContent)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

# Find conan-generated package descriptions
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
                TLS_VERIFY ON)
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.41.0 REQUIRED)

conan_cmake_configure(
    REQUIRES
        catch2/2.13.7
        aws-sdk-cpp/1.9.100
    GENERATORS
        cmake_find_package
        cmake_paths
    OPTIONS
        aws-sdk-cpp:fPIC=True
        aws-sdk-cpp:access-management=False
        aws-sdk-cpp:identity-management=False
        aws-sdk-cpp:monitoring=False
        aws-sdk-cpp:queues=False
        aws-sdk-cpp:s3-encryption=False
        aws-sdk-cpp:s3=True
        aws-sdk-cpp:text-to-speech=False
        aws-sdk-cpp:transfer=False
)

conan_cmake_autodetect(FAABRIC_CONAN_SETTINGS)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD outdated
                    REMOTE conancenter
                    PROFILE_HOST ${CMAKE_CURRENT_LIST_DIR}/../faabric/conan-profile.txt
                    PROFILE_BUILD ${CMAKE_CURRENT_LIST_DIR}/../faabric/conan-profile.txt
                    SETTINGS ${FAABRIC_CONAN_SETTINGS}
)

include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

find_package(Catch2 REQUIRED)

# There are some AWS docs on using the cpp sdk as an external project:
# https://github.com/aws/aws-sdk-cpp/blob/main/Docs/CMake_External_Project.md
# but they don't specify how to link the libraries, which required adding an
# extra couple of CMake targets.
# We now use the Conan package for aws-sdk-cpp: https://conan.io/center/aws-sdk-cpp
# To enable only S3, all default targets from the Conan recipe
# have to be explicitly disabled above
find_package(AWSSDK REQUIRED)

# Tightly-coupled dependencies
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "faasm"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT="
)

FetchContent_Declare(wamr_ext
    GIT_REPOSITORY "https://github.com/faasm/wasm-micro-runtime"
    GIT_TAG "5ac9493230902dd6ffdcbef0eeb6d5cc20fa81df"
)

# WAMR and WAVM both link to LLVM
# If WAVM is not linked statically like WAMR, there are some obscure
# static constructor errors in LLVM due to double-registration
set(WAVM_ENABLE_STATIC_LINKING ON CACHE INTERNAL "")

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

