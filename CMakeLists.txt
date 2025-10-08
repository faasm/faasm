cmake_minimum_required(VERSION 3.18.2)
project(faasm)

# Optional functionality
option(FAASM_STATIC_LIBS "Statically link Faasm libs" ON)

if (FAASM_STATIC_LIBS)
    set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
endif ()

# Performance functionality
option(FAASM_CODE_COVERAGE "Build Faasm with code coverage profiling" OFF)
option(FAASM_SELF_TRACING "Turn on system tracing using the logger" OFF)

# This option customises the SGX features _provided_ SGX is found:
option(FAASM_SGX_MODE "Type of SGX support: Disabled, Simulation or Hardware" "Simulation")

option(FAASM_TARGET_CPU "CPU to optimise for, e.g. skylake, icelake or native" OFF)

# Top-level CMake config
set(CMAKE_CXX_FLAGS "-Wall -Werror=vla -Werror=unused-variable")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")

if(FAASM_TARGET_CPU)
    message(STATUS "Optimising Faasm for CPU ${FAASM_TARGET_CPU}")
    add_compile_options(-march=${FAASM_TARGET_CPU} -mtune=${FAASM_TARGET_CPU})
else()
    message(STATUS "Faasm not optimised for specific CPU")
endif()

# Faasm profiling
if (${FAASM_SELF_TRACING})
    message("-- Activated FAASM tracing")
    add_definitions(-DTRACE_ALL=1)
    set(FAABRIC_SELF_TRACING 1)
endif ()

if (FAASM_CODE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()

# Ensure all targets can generate readable stacktraces
add_compile_options(-fno-omit-frame-pointer)
add_link_options(-Wl,--export-dynamic)

# Set-up use of sanitisers
set(WAVM_ENABLE_ASAN OFF CACHE BOOL "" FORCE)
set(WAVM_ENABLE_UBSAN OFF CACHE BOOL "" FORCE)
set(WAVM_ENABLE_TSAN OFF CACHE BOOL "" FORCE)

# Disable SGX when building with sanitisers due to linking problems
if (FAASM_USE_SANITISER STREQUAL "Address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(WAVM_ENABLE_ASAN ON CACHE BOOL "" FORCE)
    set(FAASM_SGX_MODE "Disabled")
elseif (FAASM_USE_SANITISER STREQUAL "Thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fsanitize-ignorelist=${CMAKE_CURRENT_LIST_DIR}/thread-sanitizer-ignorelist.txt")
    set(WAVM_ENABLE_TSAN ON CACHE BOOL "" FORCE)
    set(FAASM_SGX_MODE "Disabled")
elseif (FAASM_USE_SANITISER STREQUAL "Undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(WAVM_ENABLE_UBSAN ON CACHE BOOL "" FORCE)
    set(FAASM_SGX_MODE "Disabled")
elseif (FAASM_USE_SANITISER STREQUAL "Leak")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize-ignorelist=${CMAKE_CURRENT_LIST_DIR}/leak-sanitizer-ignorelist.txt")
    set(FAASM_SGX_MODE "Disabled")
elseif (FAASM_USE_SANITISER STREQUAL "Memory")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory")
    set(FAASM_SGX_MODE "Disabled")
elseif (NOT ((FAASM_USE_SANITISER STREQUAL "None") OR (NOT FAASM_USE_SANITISER)))
    message(FATAL_ERROR "Invalid FAASM_USE_SANITISER setting: ${FAASM_USE_SANITISER}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Faasm directories
set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(FAASM_NATIVE_INSTALL "/usr/local/faasm/native")

# ----------------------------------------
# Faabric
# ----------------------------------------

set(FAABRIC_TARGET_CPU ${FAASM_TARGET_CPU} CACHE BOOL "" FORCE)
add_subdirectory(faabric)

# ----------------------------------------
# SGX configuration
# ----------------------------------------

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
if (NOT (FAASM_SGX_MODE STREQUAL "Disabled"))
    # Set macro definitions for conditional compilation
    if (FAASM_SGX_MODE STREQUAL "Hardware")
        add_definitions(-DFAASM_SGX_HARDWARE_MODE)
    elseif (FAASM_SGX_MODE STREQUAL "Simulation")
        add_definitions(-DFAASM_SGX_SIMULATION_MODE)
    else ()
        message(FATAL_ERROR "Unrecognised SGX mode: ${FAASM_SGX_MODE}")
    endif ()

    # Detect SGX intallation
    find_package(SGX)
    if (NOT SGX_FOUND)
        message(FATAL_ERROR "FAASM SGX in mode ${FAASM_SGX_MODE} but SGX not found!")
    endif ()
else ()
    add_definitions(-DFAASM_SGX_DISABLED_MODE)
endif ()
message(STATUS "FAASM SGX Mode: ${FAASM_SGX_MODE}")

# ----------------------------------------
# WAMR configuration
# ----------------------------------------

add_definitions(-DBUILD_TARGET_X86_64)
add_definitions(-DWAMR_EXECUTION_MODE_INTERP=0)

# ----------------------------------------
# Faasm configuration
# ----------------------------------------

# Library type (for shared libraries)
if (FAASM_STATIC_LIBS)
    set(FAABRIC_STATIC_LIBS ON CACHE BOOL "Faabric static")
    function(faasm_public_lib lib_name)
        add_library(${lib_name} STATIC ${ARGN})
        target_link_libraries(${lib_name} PUBLIC faasm::common_deps)
        add_library(faasm::${lib_name} ALIAS ${lib_name})
    endfunction()
    function(faasm_private_lib lib_name)
        add_library(${lib_name} STATIC ${ARGN})
        target_link_libraries(${lib_name} PUBLIC faasm::common_deps)
        add_library(faasm::${lib_name} ALIAS ${lib_name})
    endfunction()
else ()
    set(FAABRIC_STATIC_LIBS OFF CACHE BOOL "Faabric static")
    function(faasm_public_lib lib_name)
        add_library(${lib_name} SHARED ${ARGN})
        target_link_options(${lib_name} PRIVATE "-fuse-ld=lld")
        target_link_libraries(${lib_name} PUBLIC faasm::common_deps)
        add_library(faasm::${lib_name} ALIAS ${lib_name})
    endfunction()
    function(faasm_private_lib lib_name)
        add_library(${lib_name} STATIC ${ARGN})
        target_link_libraries(${lib_name} PUBLIC faasm::common_deps)
        target_compile_options(${lib_name} PRIVATE "-fPIC")
        add_library(faasm::${lib_name} ALIAS ${lib_name})
    endfunction()
endif ()

# We have to be very careful here to explicitly state which LLVM
# installation we are linking against, as there may be more than one on the
# system
set(LLVM_DIR "/usr/lib/llvm-${FAASM_LLVM_MAJOR_VERSION}/cmake" CACHE STRING "" FORCE)
find_package(LLVM REQUIRED CONFIG)

# Separate LLVM vars into CMake-friendly lists
separate_arguments(LLVM_DEFINITIONS)
separate_arguments(LLVM_INCLUDE_DIRS)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} (${LLVM_INCLUDE_DIRS})")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Third party deps
include(cmake/ExternalProjects.cmake)

# See https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/build_wamr.md
set(WAMR_SHARED_DIR ${WAMR_ROOT_DIR}/core/shared)
set(WAMR_IWASM_DIR ${WAMR_ROOT_DIR}/core/iwasm)
set(WAMR_BUILD_TARGET X86_64)
set(WAMR_NO_LLVM_IMPORT ON)

add_library(faasm_common_deps INTERFACE)
add_library(faasm::common_deps ALIAS faasm_common_deps)
target_link_libraries(faasm_common_deps INTERFACE
    faabric::faabric
)
target_include_directories(faasm_common_deps INTERFACE
    # WAMR includes
    ${WAMR_SHARED_DIR}/include
    ${WAMR_SHARED_DIR}/utils
    ${WAMR_IWASM_DIR}/include
    ${WAMR_IWASM_DIR}/common
    # Faasm
    ${FAASM_INCLUDE_DIR}
    # Put the generated include directories after the source include
    # directories so that the latter take precedence
    ${FAASM_GENERATED_INCLUDE_DIR}
    ${FAASM_PYTHON_LIB_DIR}
)

# Faasm SGX support
if (SGX_FOUND)
    add_subdirectory(src/enclave)
else ()
    add_library(faasm_enclave_dummy INTERFACE)
    add_library(faasm::enclave ALIAS faasm_enclave_dummy)
endif ()

# Faasm runtime
add_subdirectory(src/codegen)
add_subdirectory(src/conf)
add_subdirectory(src/faaslet)
add_subdirectory(src/runner)
add_subdirectory(src/storage)
add_subdirectory(src/system)
add_subdirectory(src/threads)
add_subdirectory(src/upload)
add_subdirectory(src/wasm)
add_subdirectory(src/wamr)
add_subdirectory(src/wavm)

# Tests
add_subdirectory(tests/test)
add_subdirectory(tests/dist)
add_subdirectory(tests/utils)
