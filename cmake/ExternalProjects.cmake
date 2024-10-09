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
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.63.0 REQUIRED)

# Enable revisions in the conan config
execute_process(COMMAND ${CONAN_CMD} config set general.revisions_enabled=1
                RESULT_VARIABLE RET_CODE)
if(NOT "${RET_CODE}" STREQUAL "0")
    message(FATAL_ERROR "Error setting revisions for Conan: '${RET_CODE}'")
endif()

conan_cmake_configure(
    REQUIRES
        "catch2/2.13.9@#8793d3e6287d3684201418de556d98fe"
        # These three dependencies are only needed to perform remote attestation
        # of SGX enclaves using Microsoft Azure's Attestation Service
        "cppcodec/0.2@#f6385611ce2f7cff954ac8b16e25c4fa"
        "jwt-cpp/0.7.0@#c4df431e5f51bce435c3e696dcac15f8"
        "picojson/cci.20210117@#2af3ad146959275c97a6957b87b9073f"
        # 26/04/2023 - Temporarily add RapidJSON as a CMake dependency, as
        # it was removed from faabric. Eventually consolidate to just using one
        # JSON (de-)serialising library
        "rapidjson/cci.20211112@#65b4e5feb6f1edfc8cbac0f669acaf17"
    GENERATORS
        cmake_find_package
        cmake_paths
    OPTIONS
        cpprestsdk:with_websockets=False
)

conan_cmake_autodetect(FAABRIC_CONAN_SETTINGS)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD outdated
                    UPDATE
                    REMOTE conancenter
                    PROFILE_HOST ${CMAKE_CURRENT_LIST_DIR}/../faabric/conan-profile.txt
                    PROFILE_BUILD ${CMAKE_CURRENT_LIST_DIR}/../faabric/conan-profile.txt
                    SETTINGS ${FAABRIC_CONAN_SETTINGS}
)

include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

find_package(Catch2 REQUIRED)
find_package(cppcodec REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(picojson REQUIRED)
find_package(RapidJSON REQUIRED)

# Tightly-coupled dependencies
set(FETCHCONTENT_QUIET OFF)

# minio-cpp has no Conan recipe and can only be installed using VCPKG. We
# fork it and tweak the CMake file for a smoother integration.
# Main library target: miniocpp::miniocpp
FetchContent_Declare(miniocpp_ext
    GIT_REPOSITORY "https://github.com/faasm/minio-cpp"
    GIT_TAG "76e5ffd49a7d4637cb4c2194717217b74bc0a33e"
)
FetchContent_MakeAvailable(miniocpp_ext)

FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "6f4a663826f41d87d43203c9747253f8ecb3a1c0"
    CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT="
)

FetchContent_Declare(wamr_ext
    GIT_REPOSITORY "https://github.com/faasm/wasm-micro-runtime"
    # GIT_TAG "16db8a3bb11b585728608d50b38377cc75520a72"
    GIT_TAG "4e687509890f7a51bb6797f9ddace9e507ddf6dc"
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
