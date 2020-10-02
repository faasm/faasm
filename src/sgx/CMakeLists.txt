# --------------------------------------------------------
# FAASM-SGX CMake configuration file
# Before modifying please read /docs/sgx.md!
# This file contains many low level settings for Faasm-SGX
# --------------------------------------------------------

# NASM configuration
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS -felf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>")

# FAASM-SGX enclave configuration
set(FAASM_SGX_DEBUG 1) # Enable(1)/Disable(0) Faasm-SGX specific debug/error messages (Disable for better performance)
set(FAASM_SGX_INIT_TCS_SLOTS 2)
set(FAASM_SGX_ENCLAVE_PATH "sgx_wamr_enclave.sign.so")
set(FAASM_SGX_WAMR_BUILDIN_LIBC 1)
set(FAASM_SGX_WAMR_WASI_LIBC 0)

# SGX configuration
set(SGX_DEBUG_MODE 1)
set(SGX_SDK_LIB_PATH ${SGX_SDK_PATH}/lib64)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)
set(SGX_C_GLOBAL_FLAGS -m64)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)

# WAMR Build configuration
set(WAMR_BUILD_TARGET X86_64)

if(FAASM_SGX_WAMR_AOT_MODE)
    add_definitions(-DFAASM_SGX_WAMR_AOT_MODE=1)
    include (${WAMR_IWASM_DIR}/aot/iwasm_aot.cmake)
else()
    add_definitions(-DFAASM_SGX_WAMR_AOT_MODE=0)
    include (${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)
endif()

if(FAASM_SGX_WAMR_BUILDIN_LIBC)
    include (${WAMR_IWASM_DIR}/libraries/libc-builtin/libc_builtin.cmake)
endif()

if(FAASM_SGX_WAMR_WASI_LIBC)
    set(WAMR_BUILD_LIBC_WASI 1)
    include (${WAMR_IWASM_DIR}/libraries/libc-wasi/libc_wasi.cmake)
endif()

include (${WAMR_SHARED_DIR}/platform/linux-sgx/shared_platform.cmake)
include (${WAMR_SHARED_DIR}/mem-alloc/mem_alloc.cmake)
include (${WAMR_IWASM_DIR}/common/iwasm_common.cmake)
include (${WAMR_SHARED_DIR}/utils/shared_utils.cmake)

set(FAASM_SGX_WAMR_SRC
        ${IWASM_AOT_SOURCE}
        ${IWASM_COMMON_SOURCE}
        ${IWASM_INTERP_SOURCE}
        ${LIBC_BUILTIN_SOURCE}
        ${LIBC_WASI_SOURCE}
        ${MEM_ALLOC_SHARED_SOURCE}
        ${PLATFORM_SHARED_SOURCE}
        ${UTILS_SHARED_SOURCE}
        )

set(FAASM_SGX_WAMR_SRC_UNTRUSTED
        ${PLATFORM_SHARED_SOURCE_UNTRUSTED}
        )

# Attestation configuration
if (FAASM_SGX_ATTESTATION)
    add_definitions(
            -DFAASM_SGX_ATTESTATION_HOST="localhost" #IP-address or hostname is possible
            -DFAASM_SGX_ATTESTATION_PORT=8009
            -DFAASM_SGX_ATTESTATION_CALLBACK_STORE_INIT_LEN=4
    )
endif ()

# eXtended Remote Attestation XRA configuration
if(FAASM_SGX_XRA)
    add_definitions(-DFAASM_SGX_XRA=1)
endif()

# WAMR Memory sizes
set(FAASM_SGX_WAMR_HEAP_SIZE 32768)
set(FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128)
set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE 8192)
set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE 8192)

# Create definitions if necessary
add_definitions(
        -DFAASM_SGX_DEBUG=${FAASM_SGX_DEBUG}
        -DFAASM_SGX_INIT_TCS_SLOTS=${FAASM_SGX_INIT_TCS_SLOTS}
        -DFAASM_SGX_ENCLAVE_PATH="${FAASM_SGX_ENCLAVE_PATH}"
        -DFAASM_SGX_WAMR_HEAP_SIZE=${FAASM_SGX_WAMR_HEAP_SIZE}
        -DFAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE=${FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE}
        -DFAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE=${FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE}
        -DFAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE=${FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE}
)

# Debug message
message("-- Faasm-SGX Configuration:
 FAASM-SGX Debug Mode: ${FAASM_SGX_DEBUG}
 FAASM-SGX SGX Simulation Mode: ${FAASM_SGX_SIM_MODE}
 FAASM-SGX WAMR AoT Mode: ${FAASM_SGX_WAMR_AOT_MODE}
 FAASM-SGX WAMR Buildin Libc: ${FAASM_SGX_WAMR_BUILDIN_LIBC}
 FAASM-SGX WAMR Wasi Libc: ${FAASM_SGX_WAMR_WASI_LIBC}
 FAASM-SGX ATTESTATION: ${FAASM_SGX_ATTESTATION}
 FAASM-SGX WHITELISTING: ${FAASM_SGX_WHITELISTING}
 FAASM-SGX XRA: ${FAASM_SGX_XRA}
 FAASM_SGX Enclave Path: ${FAASM_SGX_ENCLAVE_PATH}
 FAASM-SGX Enclave Init TCS Slots: ${FAASM_SGX_INIT_TCS_SLOTS}
 FAASM-SGX WAMR_HEAP_SIZE: ${FAASM_SGX_WAMR_HEAP_SIZE}KB
 FAASM-SGX WAMR_MODULE_ERROR_BUFFER_SIZE: ${FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE}B
 FAASM-SGX WAMR_INSTANCE_DEFAULT_HEAP_SIZE: ${FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE}B
 FAASM-SGX WAMR_INSTANCE_DEFAULT_STACK_SIZE: ${FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE}B"
        )

# --------------------------------------------------------
# INITIAL CHECKS
# --------------------------------------------------------

# Check for SGX SDK
find_package(Threads REQUIRED)
if (EXISTS ${SGX_SDK_PATH})
    message(STATUS "Found SGX-SDK: TRUE")
else ()
    message(STATUS "Found SGX-SDK: FALSE")
    message(FATAL_ERROR "SGX-SDK not installed in ${SGX_SDK_PATH}")
endif ()

# SGX compilation flags
if (SGX_DEBUG_MODE)
    message(STATUS "Enclave will be build with debug information and -O0 optimization")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O0 -g)

    set(SGX_WAMR_C_FLAGS ${SGX_C_GLOBAL_FLAGS}
            -fPIC
            -Wno-attributes
            -DDEBUG
            -UNDEBUG
            -UEDEBUG
            )
else ()
    message(STATUS "Enclave will be build without debug information and -O2 optimization")
    set(SGX_C_GLOBAL_FLAGS ${SGX_C_GLOBAL_FLAGS} -O2)

    set(SGX_WAMR_C_FLAGS ${SGX_C_GLOBAL_FLAGS}
            -fPIC
            -Wno-attributes
            )

endif ()

# Note - these are the same in debug/ non-debug mode
set(SGX_WAMR_ENCLAVE_C_FLAGS ${SGX_C_GLOBAL_FLAGS}
        -nostdinc
        -fvisibility=hidden
        -fpie
        -ffunction-sections
        -fdata-sections
        -fstack-protector-strong
        )

# --------------------------------------------------------
# Global include for all builds
# --------------------------------------------------------

include_directories(${FAASM_INCLUDE_DIR}/sgx)

# --------------------------------------------------------
# ENCLAVE BUILD
# --------------------------------------------------------

# SGX WAMR enclave library
set(WAMR_ENCLAVE_SRC
        sgx_wamr_enclave.cpp
        sgx_wamr_native_symbols_wrapper.cpp
        rw_lock.cpp
        )

if (FAASM_SGX_WHITELISTING)
    set(WAMR_ENCLAVE_SRC ${WAMR_ENCLAVE_SRC} sgx_wamr_whitelisting.cpp)
endif ()

add_library(sgx_wamr_enclave SHARED ${WAMR_ENCLAVE_SRC})
add_dependencies(sgx_wamr_enclave wamr_sgx)

set_target_properties(sgx_wamr_enclave PROPERTIES PREFIX "")
target_include_directories(sgx_wamr_enclave PRIVATE
        ${SGX_SDK_PATH}/include
        ${SGX_SDK_PATH}/include/tlibc
        ${SGX_SDK_PATH}/include/libcxx
        ${WAMR_ROOT_DIR}/core
        ${WAMR_ROOT_DIR}/core/shared/utils
        ${WAMR_ROOT_DIR}/core/shared/platform/linux-sgx
        )
# Set XRA includes if needed
if(FAASM_SGX_XRA)
target_include_directories(sgx_wamr_enclave PRIVATE
        ${FAASM_XRA_ROOT_DIR}/include
        )
endif()

target_link_directories(sgx_wamr_enclave PRIVATE ${SGX_SDK_LIB_PATH})
target_compile_options(sgx_wamr_enclave PRIVATE
        ${SGX_WAMR_ENCLAVE_C_FLAGS}
        -std=c++11
        -nostdinc++
        )

target_link_options(sgx_wamr_enclave PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/sgx_wamr_enclave_t.o
        ${SGX_C_GLOBAL_FLAGS}
        -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles
        -Bstatic -Bsymbolic
        -Wl,-pie,-eenclave_entry
        -Wl,--export-dynamic
        -Wl,--defsym,__ImageBase=0
        -Wl,--gc-sections
        -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/sgx_wamr_enclave.lds
        )

if (FAASM_SGX_SIM_MODE)
    target_link_libraries(sgx_wamr_enclave
            -Wl,--whole-archive sgx_trts_sim
            -Wl,--no-whole-archive
            -Wl,--start-group
            sgx_tservice_sim
            -Wl,--end-group
            )
else ()
    target_link_libraries(sgx_wamr_enclave
            -Wl,--whole-archive sgx_trts
            -Wl,--no-whole-archive
            -Wl,--start-group
            sgx_tservice
            -Wl,--end-group
            )
endif ()

# Common libraries
target_link_libraries(sgx_wamr_enclave
        -Wl,--no-whole-archive
        -Wl,--start-group
        sgx_tstdc
        sgx_tcxx
        sgx_tcrypto
        wamr_sgx
        -Wl,--end-group
        )

# Enclave + XRA
if (FAASM_SGX_XRA)
    add_dependencies(sgx_wamr_enclave xra_t)

    target_link_libraries(sgx_wamr_enclave
            -Wl,--no-whole-archive
            -Wl,--start-group
            xra_t
            -Wl,--end-group
            )
endif ()

# --------------------------------------------------------
# FAASM SGX WAMR MODULE
# --------------------------------------------------------

# SGX WAMR library
set(SGX_WAMR_ASM_SRC
        SGXWAMRWasmModule.S
        )
set_source_files_properties(${SGX_WAMR_ASM_SRC}
        PROPERTIES LANGUAGE ASM_NASM
        )

set(SGX_WAMR_SRC
        SGXWAMRWasmModule.cpp
        sgx_system.cpp
        sgx_wamr_native_symbols.cpp
        )

if (FAASM_SGX_ATTESTATION)
    set(SGX_WAMR_SRC ${SGX_WAMR_SRC}
            sgx_wamr_attestation.cpp
            rw_lock.cpp
            )
endif ()


add_library(sgx_wamr STATIC
        ${SGX_WAMR_SRC}
        ${SGX_WAMR_ASM_SRC}
        )

add_dependencies(sgx_wamr sgx_wamr_enclave wamr_sgx_untrusted wasm)

target_include_directories(sgx_wamr PUBLIC ${SGX_SDK_PATH}/include)
target_link_directories(sgx_wamr INTERFACE ${SGX_SDK_LIB_PATH})
target_compile_options(sgx_wamr PRIVATE
        ${SGX_WAMR_C_FLAGS}
        -std=c++11
        -ffunction-sections
        -fdata-sections
        )

target_link_options(sgx_wamr PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/sgx_wamr_enclave_u.o
        -Wl,--gc-sections
        )

if (FAASM_SGX_SIM_MODE)
    target_link_libraries(sgx_wamr
            sgx_urts_sim
            sgx_uae_service_sim
            )
else ()
    target_link_libraries(sgx_wamr
            sgx_urts
            sgx_uae_service
            )
endif ()

target_link_libraries(sgx_wamr
        Threads::Threads
        wamr_sgx_untrusted
        wasm
        )

# --------------------------------------------------------
# WAMR Runtime Libraries
# --------------------------------------------------------

add_library(wamr_sgx STATIC
        ${FAASM_SGX_WAMR_SRC}
        )
target_compile_options(wamr_sgx PRIVATE
        -std=gnu99
        -fPIC
        -ffunction-sections
        -fdata-sections
        -Wall
        -Wno-unused-parameter
        -Wno-pedantic)
target_include_directories(wamr_sgx PRIVATE
        ${SGX_SDK_PATH}/include
        ${SGX_SDK_PATH}/include/tlibc
        ${SGX_SDK_PATH}/include/libcxx
        )

add_library(wamr_sgx_untrusted STATIC
        ${FAASM_SGX_WAMR_SRC_UNTRUSTED}
        )
target_compile_options(wamr_sgx_untrusted PRIVATE
        -fPIC
        -ffunction-sections
        -fdata-sections)

# --------------------------------------------------------
# FAASM SGX Runner
# --------------------------------------------------------
add_executable(sgx_runner sgx_runner.cpp)
add_dependencies(sgx_runner sgx_wamr)

target_compile_options(sgx_runner PRIVATE
        -O3
        -fpic
        -fpie
        -ffunction-sections
        -fdata-sections
        -Wno-attributes)

target_link_options(sgx_runner PRIVATE
        --Wl, -gc-sections)
target_link_libraries(sgx_runner PRIVATE
        sgx_wamr
        system
        )

# --------------------------------------------------------
# DEMO KEY MANAGER
# --------------------------------------------------------
add_executable(demo_keymgr demo_keymgr.c)

target_include_directories(demo_keymgr PRIVATE ${SGX_SDK_PATH}/include)
target_link_options(demo_keymgr PRIVATE -pthread)
target_compile_options(demo_keymgr PRIVATE
        -O3
        -fpic
        -fpie
        -DDEMO_KEYMGR=1)

# --------------------------------------------------------
# CUSTOM COMMANDS
# --------------------------------------------------------

# WAMR enclave
add_custom_command(TARGET sgx_wamr_enclave
        PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
        --trusted sgx_wamr_enclave.edl
        --search-path ${FAASM_SOURCE_DIR}/sgx
        --search-path ${SGX_SDK_PATH}/include
        )

add_custom_command(TARGET sgx_wamr_enclave
        PRE_BUILD COMMAND gcc
        ${SGX_WAMR_ENCLAVE_C_FLAGS}
        -I${SGX_SDK_PATH}/include
        -I${SGX_SDK_PATH}/include/tlibc
        -c sgx_wamr_enclave_t.c
        -o sgx_wamr_enclave_t.o
        )

add_custom_command(TARGET sgx_wamr_enclave
        POST_BUILD COMMAND
        ${SGX_SDK_ENCLAVE_SIGNER} sign
        -key ${CMAKE_CURRENT_SOURCE_DIR}/sgx_wamr_enclave.pem
        -enclave ${CMAKE_BINARY_DIR}/lib/sgx_wamr_enclave.so
        -out ${CMAKE_BINARY_DIR}/bin/sgx_wamr_enclave.sign.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/sgx_wamr_enclave.config
        )

# SGX-WAMR
add_custom_command(TARGET sgx_wamr
        PRE_BUILD COMMAND ${SGX_SDK_ENCLAVE_EDGER8R}
        --untrusted sgx_wamr_enclave.edl
        --search-path ${FAASM_SOURCE_DIR}/sgx
        --search-path ${SGX_SDK_PATH}/include
        )

add_custom_command(TARGET sgx_wamr
        PRE_BUILD COMMAND gcc
        ${SGX_WAMR_C_FLAGS}
        -I${SGX_SDK_PATH}/include
        -c sgx_wamr_enclave_u.c
        -o sgx_wamr_enclave_u.o
        )
