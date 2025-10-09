include(FindGit)
find_package(Git)
include (ExternalProject)
include (FetchContent)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

# Find conan-generated package descriptions
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(Catch2 REQUIRED)
find_package(cppcodec REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(picojson REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

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
    GIT_TAG "1bf99ccebaf40390c7366ffadac8113d4b9d425f"
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
