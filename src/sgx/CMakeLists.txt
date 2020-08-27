# --------------------------------------------------------
# BUILD CONFIGURATION
# --------------------------------------------------------

message(- "CURRENT BINARY DIR: ${CMAKE_CURRENT_BINARY_DIR}")

# Include WAMR SGX platform
include_directories(${WAMR_SHARED_DIR}/platform/linux-sgx/)

# Flags to switch on/ off WAMR interpreter and AOT
if (WAMR_BUILD_INTERP)
    add_definitions(-DWASM_ENABLE_INTERP=1)
else ()
    add_definitions(-DWASM_ENABLE_INTERP=0)
endif ()
if (WAMR_BUILD_AOT)
    add_definitions(-DWASM_ENABLE_AOT=1)
else ()
    add_definitions(-DWASM_ENABLE_AOT=0)
endif ()

# NASM configuration
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS -felf64)
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> ${CMAKE_ASM_NASM_FLAGS} -o <OBJECT> <SOURCE>")

# SGX config
set(SGX_DEBUG_MODE 1)
set(SGX_SDK_LIB_PATH ${SGX_SDK_PATH}/lib64)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)
set(SGX_C_GLOBAL_FLAGS -m64)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(SGX_SDK_ENCLAVE_SIGNER ${SGX_SDK_PATH}/bin/x64/sgx_sign)
set(SGX_SDK_ENCLAVE_EDGER8R ${SGX_SDK_PATH}/bin/x64/sgx_edger8r)

# Attestation config
if (FAASM_SGX_ATTESTATION)
    add_definitions(
            -DFAASM_SGX_ATTESTATION_HOST="localhost"
            -DFAASM_SGX_ATTESTATION_PORT=8009
    )
    add_definitions(
            -DFAASM_SGX_ATTESTATION_CALLBACK_STORE_INIT_LEN=4
    )
endif ()

# Memory sizes
set(FAASM_SGX_WAMR_HEAP_SIZE 32768)
set(FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE 128)
set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE 8192)
set(FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE 8192)

# Debug message
message("-- SGX-WAMR Configuration:
 FAASM_SGX_WAMR_HEAP_SIZE: ${FAASM_SGX_WAMR_HEAP_SIZE}KB
 FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE: ${FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE}B
 FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE: ${FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE}B
 FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE: ${FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE}B
 FAASM_SGX_ATTESTATION: ${FAASM_SGX_ATTESTATION}
 FAASM_SGX_WHITELISTING: ${FAASM_SGX_WHITELISTING}"
        )

add_definitions(
        -DFAASM_SGX_WAMR_HEAP_SIZE=${FAASM_SGX_WAMR_HEAP_SIZE}
        -DFAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE=${FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE}
        -DFAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE=${FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE}
        -DFAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE=${FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE}
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

# Check for SGX simulation
if (FAASM_SGX_SIM_MODE)
    message(STATUS "Enclave will be build in simulation mode")
else ()
    message(STATUS "Enclave will be build in hardware mode")
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

set_target_properties(sgx_wamr_enclave PROPERTIES PREFIX "")
target_include_directories(sgx_wamr_enclave PRIVATE
        ${SGX_SDK_PATH}/include
        ${SGX_SDK_PATH}/include/tlibc
        ${SGX_SDK_PATH}/include/libcxx
        ${WAMR_ROOT_DIR}/core
        ${WAMR_ROOT_DIR}/core/shared/utils
        ${WAMR_ROOT_DIR}/core/shared/platform/linux-sgx
        )

target_link_directories(sgx_wamr_enclave PRIVATE ${SGX_SDK_LIB_PATH})
target_compile_options(sgx_wamr_enclave PRIVATE
        ${SGX_WAMR_ENCLAVE_C_FLAGS} -std=c++11 -nostdinc++
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
            sgx_tstdc
            sgx_tcxx
            sgx_tcrypto
            sgx_tservice_sim
            wamrlib_sgx
            -Wl,--end-group
            )
else ()
    target_link_libraries(sgx_wamr_enclave
            -Wl,--whole-archive sgx_trts
            -Wl,--no-whole-archive
            -Wl,--start-group
            sgx_tstdc
            sgx_tcxx
            sgx_tcrypto
            sgx_tservice
            wamrlib_sgx
            -Wl,--end-group
            )
endif ()

# Enclave + XRA
if (SGX_XRA)
    add_dependencies(sgx_wamr_enclave xra_t)

    target_link_libraries(sgx_wamr_enclave
            -Wl,--no-whole-archive
            -Wl,--start-group xra_t -Wl,--end-group
            )
endif ()

# --------------------------------------------------------
# FAASM SGX WAMR MODULE
# --------------------------------------------------------

# SGX WAMR library
set(SGX_WAMR_ASM_SOURCE_FILES
        SGXWAMRWasmModule.S
        )
set_source_files_properties(${SGX_WAMR_ASM_SOURCE_FILES} PROPERTIES LANGUAGE ASM_NASM)

set(SGX_WAMR_SRC
        SGXWAMRWasmModule.cpp
        sgx_wamr_native_symbols.cpp
        rw_lock.cpp
        )

if (FAASM_SGX_WHITELISTING)
    set(SGX_WAMR_SRC ${SGX_WAMR_SRC}
            sgx_wamr_whitelisting.cpp
            )
endif ()

if (FAASM_SGX_ATTESTATION)
    set(SGX_WAMR_SRC ${SGX_WAMR_SRC}
            sgx_wamr_attestation.cpp
            )
endif ()


add_library(sgx_wamr STATIC
        ${SGX_WAMR_SRC}
        ${SGX_WAMR_ASM_SOURCE_FILES}
        )

add_dependencies(sgx_wamr sgx_wamr_enclave wamrlib_sgx wasm)

target_include_directories(sgx_wamr PUBLIC ${SGX_SDK_PATH}/include)
target_link_directories(sgx_wamr INTERFACE ${SGX_SDK_LIB_PATH})
target_compile_options(sgx_wamr PRIVATE ${SGX_WAMR_C_FLAGS} -std=c++11)

# Here we need to force sgx_wamr to link with the output of the
# enclave build.
#set_target_properties(sgx_wamr PROPERTIES STATIC_LIBRARY_FLAGS
#        ${CMAKE_CURRENT_BINARY_DIR}/sgx_wamr_enclave_u.o
#        )

target_link_options(sgx_wamr PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/sgx_wamr_enclave_u.o
        )

target_link_libraries(sgx_wamr
        Threads::Threads
        wamrlib_sgx
        wasm
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

# SGX runner
include_directories(${FAASM_INCLUDE_DIR}/sgx)
add_executable(sgx_runner sgx_runner.cpp)
add_dependencies(sgx_runner sgx_wamr)

target_compile_options(sgx_runner PRIVATE -O3 -fpic -Wno-attributes)

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
target_compile_options(demo_keymgr PRIVATE -O3 -DDEMO_KEYMGR=1)

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
