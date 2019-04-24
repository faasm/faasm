cmake_minimum_required(VERSION 3.8.0)
project(faasm)

set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Address sanitizer
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -O0")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Oz")

set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
set(FAASM_PYTHON_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/python)
set(FAASM_LAMBDA_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lambda)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/WAVM/Lib)

# Stops WAVM compilation complaining
add_definitions(-DDLL_EXPORT=)
add_definitions(-DDLL_IMPORT=)

# Turn off eigen parallelisation
# add_definitions(-DEIGEN_DONT_PARALLELIZE)
# add_definitions(-DEIGEN_NO_MALLOC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# AWS Lambda runtime (used by all builds)
find_package(aws-lambda-runtime)

# AWS SDK
find_package(AWSSDK REQUIRED COMPONENTS lambda s3 sqs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_LIB_DIR}
        ${FAASM_PYTHON_LIB_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/src # Required for protobuf
)

if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    message(STATUS "Running lambda function build")

    add_subdirectory(func)
    add_subdirectory(lib)

    add_subdirectory(src/aws)
    add_subdirectory(src/lambda_func)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-redis")
    message(STATUS "Running lambda redis build")

    add_subdirectory(src/aws)
    add_subdirectory(src/lambda)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-state")
    message(STATUS "Running lambda state build")

    add_subdirectory(src/aws)
    add_subdirectory(src/lambda)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/state)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-worker")
    # ------------------
    # Lambda worker build
    # ------------------

    message(STATUS "Running lambda worker build")

    add_subdirectory(WAVM)

    add_subdirectory(src/aws)
    add_subdirectory(src/lambda)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
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
    add_subdirectory(src/lambda)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/util)

else ()
    # ------------------
    # Standard build
    # ------------------

    # WAVM
    add_subdirectory(WAVM)

    # Faasm functions
    add_subdirectory(func)

    # Faasm libraries
    add_subdirectory(lib)
    add_subdirectory(python)

    # Custom malloc
    add_subdirectory(malloc)

    # Experiments
    add_subdirectory(experiment)

    # Faasm runtime
    add_subdirectory(src/aws)
    add_subdirectory(src/data)
    add_subdirectory(src/dispatch)
    add_subdirectory(src/edge)
    add_subdirectory(src/emulator)
    add_subdirectory(src/lambda)
    add_subdirectory(src/lambda_func)
    add_subdirectory(src/memory)
    add_subdirectory(src/prof)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/runner)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/upload)
    add_subdirectory(src/util)
    add_subdirectory(src/wasm)
    add_subdirectory(src/worker)

    # Tests
    add_library(catch INTERFACE)
    add_subdirectory(test)
    add_subdirectory(test_integration)
endif ()