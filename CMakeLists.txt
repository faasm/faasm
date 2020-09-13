cmake_minimum_required(VERSION 3.13.0)
project(faasm)

# Optional functionality
option(FAASM_STATIC_LIBS "Statically link Faasm libs" ON)

# Performance functionality
option(FAASM_SELF_TRACING "Turn on system tracing using the logger" OFF)
option(FAASM_OPENMP_TRACING "Trace OpenMP forks in Redis" OFF)
option(FAASM_PERF_PROFILING "Turn on profiling features as described in debugging.md" OFF)

# SGX functionality
set(FAASM_SGX_SUPPORT 0) #Add support for SGX
option(FAASM_SGX_SIM_MODE "SGX sim mode flag" ON)
option(FAASM_SGX_ATTESTATION "Add support for attestation" OFF)
option(FAASM_SGX_WHITELISTING "Add support for whitelisting" OFF)
option(FAASM_SGX_XRA "Add support for XRA" OFF)

# Top-level CMake config
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Faasm directories
set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_LIB_CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/cpp)
set(FAASM_LIB_FAASMP_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/faasmp)
set(FAASM_PYTHON_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/pyinit)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/WAVM/Lib)
set(FAASM_WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/wamr)
set(FAASM_FAABRIC_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/faabric)
set(FAASM_XRA_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/xra)

# SGX configuration
if (FAASM_SGX_SUPPORT)
    # SGX includes
    set(SGX_SDK_PATH /opt/intel/sgxsdk)
    include_directories(${SGX_SDK_PATH}/include)

    # Switch off Werror only when in SGX mode
    set(CMAKE_CXX_FLAGS "-Wall")

    # Set up SGX definitions
    add_definitions(-DFAASM_SGX=1)

    if(FAASM_SGX_SIM_MODE)
        add_definitions(-DSGX_SIM_MODE=1)
    endif()

    if(FAASM_SGX_ATTESTATION)
        add_definitions(-DFAASM_SGX_ATTESTATION=1)
    endif()

    if(FAASM_SGX_WHITELISTING)
        add_definitions(-DFAASM_SGX_WHITELISTING=1)
    endif()

    if(FAASM_SGX_XRA)
        include_directories(${FAASM_XRA_ROOT_DIR}/include)
    endif()
else ()
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
    add_definitions(-DFAASM_SGX=0)
endif ()

# ----------------------------------------
# WAMR configuration
# ----------------------------------------

# See https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/build_wamr.md
set(WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/wamr)
set(WAMR_SHARED_DIR ${WAMR_ROOT_DIR}/core/shared)
set(WAMR_IWASM_DIR ${WAMR_ROOT_DIR}/core/iwasm)
set(WAMR_BUILD_TARGET X86_64)
set(WAMR_EXECUTION_MODE Interpreter) #AoT or Interpreter
set(WAMR_NO_LLVM_IMPORT ON)
add_definitions(-DBUILD_TARGET_X86_64)

# Enable ASM
enable_language(ASM)

# WAMR includes
include_directories(
        ${WAMR_SHARED_DIR}/include
        ${WAMR_SHARED_DIR}/utils
        ${WAMR_IWASM_DIR}/include
        ${WAMR_IWASM_DIR}/common
)

#WAMR Interpreter / AoT Switch
if(WAMR_EXECUTION_MODE STREQUAL "Interpreter")
    add_definitions(-DWAMR_EXECUTION_MODE_INTERP=1)
else()
    add_definitions(-DWAMR_EXECUTION_MODE_INTERP=0)
endif()

# WAMR platform header, will break SGX build to include normal linux header
if(FAASM_SGX_SUPPORT)
else()
    include_directories(${WAMR_SHARED_DIR}/platform/linux/)
endif()

# Library type (for shared libraries)
if (FAASM_STATIC_LIBS)
    set(FAABRIC_STATIC_LIBS ON CACHE BOOL "Faabric static")
    function(faasm_public_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
    endfunction()
    function(faasm_private_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
    endfunction()
else ()
    set(FAABRIC_STATIC_LIBS OFF CACHE BOOL "Faabric static")
    function(faasm_public_lib lib_name lib_deps)
        add_library(${lib_name} SHARED ${lib_deps})
    endfunction()
    function(faasm_private_lib lib_name lib_deps)
        add_library(${lib_name} STATIC ${lib_deps})
        target_compile_options(${lib_name} PRIVATE "-fPIC")
    endfunction()
endif ()

# Switch on WAVM stack traces
set(WAVM_ENABLE_UNWIND ON CACHE BOOL "WAVM unwind")

# Stops WAVM compilation complaining
add_definitions(-DDLL_EXPORT=)
add_definitions(-DDLL_IMPORT=)

# Faasm profiling
if (${FAASM_SELF_TRACING})
    message("-- Activated FAASM tracing")
    add_definitions(-DTRACE_ALL=1)
endif ()

if (${FAASM_OPENMP_TRACING})
    add_definitions(-DOPENMP_FORK_REDIS_TRACE=1)
endif ()

# LLVM config
if (${FAASM_PERF_PROFILING})
    # In accordance with bin/build_llvm_perf.sh and LLVM version for WAVM
    set(LLVM_DIR /usr/local/code/llvm-perf/build/lib/cmake/llvm)
    message(STATUS "Using custom LLVM at ${LLVM_DIR} for profiling")
    find_package(LLVM REQUIRED CONFIG NO_DEFAULT_PATH)

    set(WAVM_PERF_LIBS 1)
    add_definitions(-DWAVM_PERF_EVENTS=1)
else ()
    find_package(LLVM REQUIRED CONFIG)
endif ()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_LIB_CPP_DIR}
        ${FAASM_LIB_FAASMP_DIR}
        ${FAASM_PYTHON_LIB_DIR}
        ${FAASM_FAABRIC_ROOT_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/third-party/faabric/src # Required for protobuf
)

# Directories common to all builds
add_subdirectory(third-party/faabric)
add_subdirectory(libs/cpp)
add_subdirectory(libs/faasmp)
add_subdirectory(libs/faasmpi)
add_subdirectory(src/emulator)

if (FAASM_BUILD_TYPE STREQUAL "native-tools")
    # Ignore the rest if just doing a native tools build
else ()
    # WAVM
    add_subdirectory(third-party/WAVM)

    # Faasm functions
    add_subdirectory(func)

    # Faasm libraries
    add_subdirectory(libs/pyinit)
    add_subdirectory(libs/rust)

    # Faasm runtime
    add_subdirectory(src/faaslet)
    add_subdirectory(src/ir_cache)
    add_subdirectory(src/module_cache)
    add_subdirectory(src/runner)
    add_subdirectory(src/storage)
    add_subdirectory(src/system)
    add_subdirectory(src/upload)
    add_subdirectory(src/wasm)
    add_subdirectory(src/wamr)
    add_subdirectory(src/wavm)

    # WAMR - build SGX or non-SGX version accordingly
    if (FAASM_SGX_SUPPORT)
        add_subdirectory(src/sgx_wamr)
        add_subdirectory(src/sgx)

        if(FAASM_SGX_XRA)
            add_subdirectory(third-party/xra)
        endif()
    endif ()

    # Tests
    add_subdirectory(tests/test)
    add_subdirectory(tests/utils)
endif ()
