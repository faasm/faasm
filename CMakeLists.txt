cmake_minimum_required(VERSION 3.8.0)
project(faasm)

# Optional functionality
option(FAASM_STATIC_LIBS "Statically link Faasm libs" ON)
option(FAASM_WAMR_SUPPORT "Add support for WAMR" ON)
option(FAASM_SGX_SUPPORT "Add support for SGX" ON)

# Top-level CMake config
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Faasm directories
set(FAASM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(FAASM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(FAASM_LIB_CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/cpp)
set(FAASM_LIB_FAASMP_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/faasmp)
set(FAASM_PYTHON_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/libs/pyinit)
set(FAASM_WAVM_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/WAVM/Lib)

# SGX configuration
if(FAASM_SGX_SUPPORT)
    # Switch off Werror only when in SGX mode
    set(CMAKE_CXX_FLAGS "-Wall")

    # Set up SGX definitions
    add_definitions(-DFAASM_SGX=1 -DFAASM_SGX_FUNC_SUFFIX="_sgx_wamr")
    set(SGX_SIM_MODE 1)

    if(SGX_SIM_MODE)
        add_definitions(-DSGX_SIM_MODE=1)
    elseif()
        add_definitions(-DSGX_SIM_MODE=0)
    endif()
else()
    set(CMAKE_CXX_FLAGS "-Wall -Werror")
endif()

# WAMR configuration
if (FAASM_WAMR_SUPPORT)
    set(WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/wamr)
    set(WAMR_INCLUDE_DIR ${WAMR_ROOT_DIR}/core/iwasm/include)
    set(WAMR_BUILD_TARGET X86_64)
    set(WAMR_BUILD_INTERP 1)

    # WAMR SGX-specific config
    if(FAASM_SGX_SUPPORT)
        set(WAMR_BUILD_PLATFORM linux-sgx)
        set(WAMR_BUILD_FAST_INTERP 0)
        set(WAMR_BUILD_JIT 0)
        set(WAMR_BUILD_AOT 0)
        set(WAMR_BUILD_LIBC_WASI 0)
        set(WAMR_BUILD_LIBC_BUILTIN 1)

        set(FAASM_SGX_WAMR_HEAP_SIZE 32768)
        set(FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128)
        set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE 8192)
        set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE 8192)

    # Non-SGX WAMR config
    else(FAASM_WAMR_SUPPORT)
        set(WAMR_BUILD_PLATFORM linux)
        set(WAMR_BUILD_FAST_INTERP 1)
        set(WAMR_BUILD_JIT 1)
        set(WAMR_BUILD_AOT 0)
        set(WAMR_BUILD_LIBC_WASI 1)
        set(WAMR_BUILD_LIBC_BUILTIN 0)
    endif()
endif ()

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
option(FAASM_SELF_TRACING "Turn on system tracing using the logger")
option(FAASM_OPENMP_TRACING "Trace OpenMP forks in Redis")
option(FAASM_PERF_PROFILING "Turn on profiling features as described in debugging.md")

if (${FAASM_SELF_TRACING})
    message("-- Activated FAASM tracing")
    add_definitions(-DTRACE_ALL=1)
endif ()

if (${FAASM_OPENMP_TRACING})
    add_definitions(-DOPENMP_FORK_REDIS_TRACE=1)
endif ()

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

# Eigen parallelisation
# add_definitions(-DEIGEN_DONT_PARALLELIZE)
# add_definitions(-DEIGEN_NO_MALLOC)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        ${FAASM_INCLUDE_DIR}
        ${FAASM_LIB_CPP_DIR}
        ${FAASM_LIB_FAASMP_DIR}
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
    add_subdirectory(src/tcp)
    add_subdirectory(src/util)

elseif (FAASM_BUILD_TYPE STREQUAL "native-tools")
    # ------------------
    # Native tools for porting code
    # ------------------

    message(STATUS "Running native tools build")

    add_subdirectory(libs/cpp)
    add_subdirectory(libs/faasmp)
    add_subdirectory(src/emulator)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/tcp)
    add_subdirectory(src/util)

else ()
    # ------------------
    # Standard build
    # ------------------

    # WAVM
    add_subdirectory(third-party/WAVM)

    # WAMR
    if (FAASM_WAMR_SUPPORT)
        include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
        add_library(libwamr ${WAMR_RUNTIME_LIB_SOURCE})
    endif ()

    # SGX
    if(FAASM_SGX_SUPPORT)
        add_subdirectory(src/sgx)
    endif()

    # Faasm functions
    add_subdirectory(func)

    # Faasm libraries
    add_subdirectory(libs/cpp)
    add_subdirectory(libs/faasmp)
    add_subdirectory(libs/faasmpi)
    add_subdirectory(libs/pyinit)
    add_subdirectory(libs/rust)

    # Faasm runtime
    add_subdirectory(src/codegen)
    add_subdirectory(src/emulator)
    add_subdirectory(src/endpoint)
    add_subdirectory(src/faaslet)
    add_subdirectory(src/ibm)
    add_subdirectory(src/ir_cache)
    add_subdirectory(src/knative)
    add_subdirectory(src/knative_native)
    add_subdirectory(src/module_cache)
    add_subdirectory(src/mpi)
    add_subdirectory(src/proto)
    add_subdirectory(src/redis)
    add_subdirectory(src/runner)
    add_subdirectory(src/scheduler)
    add_subdirectory(src/state)
    add_subdirectory(src/storage)
    add_subdirectory(src/system)
    add_subdirectory(src/tcp)
    add_subdirectory(src/upload)
    add_subdirectory(src/util)
    add_subdirectory(src/wamr)
    add_subdirectory(src/wasm)
    add_subdirectory(src/wavm)

    # Tests
    add_library(catch INTERFACE)
    add_subdirectory(tests/test)
    add_subdirectory(tests/utils)
    add_subdirectory(tests/ibm)
    add_subdirectory(tests/knative)
endif ()
