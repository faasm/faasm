cmake_minimum_required(VERSION 3.8.0)
project(faasm)

# Top-level CMake config
set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Faasm directories
set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_LIB_CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/cpp)
set(FAASM_PYTHON_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/pyinit)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/WAVM/Lib)

option(FAASM_STATIC_LIBS "Statically link Faasm libs" ON)
option(FAASM_AWS_SUPPORT "Whether to include AWS support" OFF)

# Library type (for shared libraries)
if (FAASM_STATIC_LIBS)
    function(faasm_public_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
    endfunction()
    function(faasm_private_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
    endfunction()
else ()
    function(faasm_public_lib lib_name lib_deps)
        add_library(${lib_name} SHARED ${lib_deps})
    endfunction()
    function(faasm_private_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
        target_compile_options(${lib_name} PRIVATE "-fPIC")
    endfunction()
endif ()

# Switch on WAVM stack traces in debug (potential performance gain?)
set(WAVM_ENABLE_UNWIND ON CACHE BOOL "WAVM unwind")
#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    set(WAVM_ENABLE_UNWIND ON CACHE BOOL "WAVM unwind")
#else()
#    set(WAVM_ENABLE_UNWIND OFF CACHE BOOL "WAVM unwind")
#endif()

# Stops WAVM compilation complaining
add_definitions(-DDLL_EXPORT=)
add_definitions(-DDLL_IMPORT=)

# Faasm profiling
add_definitions(-DFAASM_PROFILE_ON=0)

# Custom LLVM build (also for profiling)
set(FAASM_CUSTOM_LLVM 0)
if (${FAASM_CUSTOM_LLVM})
    message(STATUS "Using custom LLVM")

    # Force using custom build of LLVM for codegen
    set(LLVM_DIR $ENV{HOME}/faasm/llvm/build/lib/cmake/llvm)
    find_package(LLVM REQUIRED CONFIG NO_DEFAULT_PATH)

    set(WAVM_PERF_LIBS 1)
    add_definitions(-DWAVM_PERF_EVENTS=1)
else ()
    # Use LLVM 8 explicitly for the lib parts (i.e. codegen)
    set(LLVM_DIR /usr/lib/llvm-8)
    find_package(LLVM REQUIRED CONFIG)
endif ()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Eigen parallelisation
# add_definitions(-DEIGEN_DONT_PARALLELIZE)
# add_definitions(-DEIGEN_NO_MALLOC)

if (FAASM_AWS_SUPPORT)
    # AWS
    find_package(aws-lambda-runtime)

    # AWS SDK
    find_package(AWSSDK REQUIRED COMPONENTS lambda s3 sqs)

    add_definitions("-DAWS_SUPPORT=1")
endif ()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_LIB_CPP_DIR}
        ${FAASM_PYTHON_LIB_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/src # Required for protobuf
)

if (FAASM_BUILD_TYPE STREQUAL "knative-native")
    message(STATUS "Running knative native build")

    add_subdirectory(func)
    add_subdirectory(libs/cpp)
    add_subdirectory(libs/pyinit)

    add_subdirectory(src/emulator)
    add_subdirectory(src/endpoint)
    add_subdirectory(src/knative_native)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    message(STATUS "Running lambda function build")

    add_subdirectory(func)
    add_subdirectory(libs/cpp)

    add_subdirectory(src/aws)
    add_subdirectory(src/lambda_func)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-redis")
    message(STATUS "Running lambda redis build")

    add_subdirectory(src/aws)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-state")
    message(STATUS "Running lambda state build")

    add_subdirectory(src/aws)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/state)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-worker")
    # ------------------
    # Lambda worker build
    # ------------------

    message(STATUS "Running lambda worker build")

    add_subdirectory(third-party/WAVM)

    add_subdirectory(src/aws)
    add_subdirectory(src/codegen)
    add_subdirectory(src/ir_cache)
    add_subdirectory(src/lambda)
    add_subdirectory(src/module_cache)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/storage)
    add_subdirectory(src/system)
    add_subdirectory(src/util)
    add_subdirectory(src/wasm)
    add_subdirectory(src/worker)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-dispatch")
    # ------------------
    # Lambda dispatch build
    # ------------------

    message(STATUS "Running lambda dispatch build")

    add_subdirectory(src/aws)
    add_subdirectory(src/dispatch)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "native-tools")
    # ------------------
    # Native tools for porting code
    # ------------------

    message(STATUS "Running native tools build")

    set(FAASM_LIB_TYPE SHARED)

    add_subdirectory(libs/cpp)
    add_subdirectory(libs/faasmp)
    add_subdirectory(src/emulator)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/util)

else ()
    # ------------------
    # Standard build
    # ------------------

    # WAVM
    add_subdirectory(third-party/WAVM)

    # Faasm demo
    add_subdirectory(third-party/faasm-demo-c)

    # Faasm functions
    add_subdirectory(func)

    # Faasm libraries
    add_subdirectory(libs/cpp)
    add_subdirectory(libs/faasmp)
    add_subdirectory(libs/faasmpi)
    add_subdirectory(libs/pyinit)
    add_subdirectory(libs/rust)

    # Custom malloc
    add_subdirectory(third-party/malloc)

    # AWS-specific
    if (FAASM_AWS_SUPPORT)
        add_subdirectory(src/aws)
        add_subdirectory(src/lambda_func)
        add_subdirectory(src/dispatch)
        add_subdirectory(src/lambda)
    endif ()

    # Faasm runtime
    add_subdirectory(src/codegen)
    add_subdirectory(src/data)
    add_subdirectory(src/emulator)
    add_subdirectory(src/endpoint)
    add_subdirectory(src/experiment)
    add_subdirectory(src/ibm)
    add_subdirectory(src/ir_cache)
    add_subdirectory(src/knative)
    add_subdirectory(src/knative_native)
    add_subdirectory(src/module_cache)
    add_subdirectory(src/mpi)
    add_subdirectory(src/openmp)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/runner)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/storage)
    add_subdirectory(src/system)
    add_subdirectory(src/upload)
    add_subdirectory(src/util)
    add_subdirectory(src/wasm)
    add_subdirectory(src/worker)

    # Tests
    add_library(catch INTERFACE)
    add_subdirectory(tests/test)
    add_subdirectory(tests/utils)
    add_subdirectory(tests/ibm)
    add_subdirectory(tests/knative)

    if (FAASM_AWS_SUPPORT)
        add_subdirectory(tests/aws)
    endif ()
endif ()
