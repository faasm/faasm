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
    GIT_REPOSITORY "https://github.com/faasm/wasm-micro-runtime"
    GIT_TAG "d4a3246b3b10a7792ca69d260d428fe21518a0b3"
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

ExternalProject_Add(catch2_ext
     GIT_REPOSITORY "https://github.com/catchorg/Catch2"
     GIT_TAG "v2.13.2"
     CMAKE_ARGS "-DCATCH_INSTALL_DOCS=OFF \
         -DCATCH_INSTALL_EXTRAS=OFF"
     CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
)

