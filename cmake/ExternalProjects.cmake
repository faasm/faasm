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

conan_check(VERSION 1.43.0 REQUIRED)

# Enable revisions in the conan config
execute_process(COMMAND ${CONAN_CMD} config set general.revisions_enabled=1
                RESULT_VARIABLE RET_CODE)
if(NOT "${RET_CODE}" STREQUAL "0")
    message(FATAL_ERROR "Error setting revisions for Conan: '${RET_CODE}'")
endif()

conan_cmake_configure(
    REQUIRES
        catch2/2.13.7@#31c8cd08e3c957a9eac8cb1377cf5863
    GENERATORS
        cmake_find_package
        cmake_paths
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

# 22/12/2021 - WARNING: we don't install AWS through Conan as the recipe proved
# very unstable and failed frequently.

# There are some AWS docs on using the cpp sdk as an external project:
# https://github.com/aws/aws-sdk-cpp/blob/main/Docs/CMake_External_Project.md
# but they don't specify how to link the libraries, which required adding an
# extra couple of CMake targets.
# There are some AWS docs on using the cpp sdk as an external project:
# https://github.com/aws/aws-sdk-cpp/blob/main/Docs/CMake_External_Project.md
# but they don't specify how to link the libraries, which required adding an
# extra couple of CMake targets.
set(AWS_CORE_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libaws-cpp-sdk-core.so)
set(AWS_S3_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/libaws-cpp-sdk-s3.so)
ExternalProject_Add(aws_ext
    GIT_REPOSITORY   "https://github.com/aws/aws-sdk-cpp.git"
    GIT_TAG          "b733384b16945818fa5da5b73e410dea1e9ab9d0"
    BUILD_ALWAYS     0
    TEST_COMMAND     ""
    UPDATE_COMMAND   ""
    BUILD_BYPRODUCTS ${AWS_S3_LIBRARY} ${AWS_CORE_LIBRARY}
    CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
    LIST_SEPARATOR    "|"
    CMAKE_ARGS       -DBUILD_SHARED_LIBS=ON
                     -DBUILD_ONLY=s3|sts
                     -DAUTORUN_UNIT_TESTS=OFF
                     -DENABLE_TESTING=OFF
                     -DCMAKE_BUILD_TYPE=Release
)

add_library(aws_ext_core SHARED IMPORTED)
add_library(aws_ext_s3 SHARED IMPORTED)
set_target_properties(aws_ext_core
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_INSTALL_PREFIX}/lib/libaws-cpp-sdk-core.so)
set_target_properties(aws_ext_s3
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_INSTALL_PREFIX}/lib/libaws-cpp-sdk-s3.so)
add_dependencies(aws_ext_core aws_ext)
add_dependencies(aws_ext_s3 aws_ext)
# Merge the two libraries in one aliased interface
add_library(aws_ext_s3_lib INTERFACE)
target_link_libraries(aws_ext_s3_lib INTERFACE aws_ext_s3 aws_ext_core)
add_library(AWS::s3 ALIAS aws_ext_s3_lib)

# Tightly-coupled dependencies
set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(wavm_ext
    GIT_REPOSITORY "https://github.com/faasm/WAVM.git"
    GIT_TAG "2434b3b170404b63afbd2103eecbf510dd1665f4"
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

