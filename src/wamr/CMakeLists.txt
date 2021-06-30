# Unfortunately the pre-canned WAMR CMake file modifies too many top-level
# variables (e.g. through use of include_directories or changing global
# CFLAGS).
#
# We have our own custom CMake set-up which looks a bit like the wamrc build:
# https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/wamr-compiler/CMakeLists.txt

# Turn on WAMR features
add_definitions(-DWASM_ENABLE_BULK_MEMORY=1)
add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)
add_definitions(-DWASM_ENABLE_MULTI_MODULE=1)

# Here we include all the components of WAMR we want
include(${WAMR_SHARED_DIR}/platform/linux/shared_platform.cmake)
include(${WAMR_SHARED_DIR}/mem-alloc/mem_alloc.cmake)
include(${WAMR_SHARED_DIR}/utils/shared_utils.cmake)
include(${WAMR_SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)
include(${WAMR_IWASM_DIR}/common/iwasm_common.cmake)
include(${WAMR_IWASM_DIR}/libraries/libc-wasi/libc_wasi.cmake)

if(FAASM_WAMR_AOT_MODE)
    # set (WAMR_BUILD_AOT 1)
    # include (${WAMR_ROOT_DIR}/build-scripts/config_common.cmake)
    include(${WAMR_IWASM_DIR}/aot/iwasm_aot.cmake)
    include(${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)
    include(${WAMR_IWASM_DIR}/compilation/iwasm_compl.cmake)
    faasm_private_lib(aotclib "${IWASM_COMPL_SOURCE}")
else()
    include(${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)
endif()

set(COMMON_WAMR_SRC
        ${PLATFORM_SHARED_SOURCE}
        ${MEM_ALLOC_SHARED_SOURCE}
        ${IWASM_COMMON_SOURCE}
        ${UTILS_SHARED_SOURCE}
        ${IWASM_INTERP_SOURCE}
        ${IWASM_AOT_SOURCE}
        ${LIBC_WASI_SOURCE}
)

# Build the WAMR lib
set(ALL_WAMR_SRC ${COMMON_WAMR_SRC} ${EXTRA_WAMR_SRC})
faasm_private_lib(wamrlib "${ALL_WAMR_SRC}")

# -----------------------------
# Faasm-specific configuration
# -----------------------------

set(PLATFORM_HEADER ${WAMR_SHARED_DIR}/platform/linux)

target_include_directories(
    wamrlib PUBLIC
    ${FAASM_INCLUDE_DIR}/wamr
    ${PLATFORM_HEADER}
    )

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
        )

set(LIB_FILES
        WAMRWasmModule.cpp
        codegen.cpp
        dynlink.cpp
        filesystem.cpp
        funcs.cpp
        memory.cpp
        native.cpp
        pthread.cpp
        state.cpp
        stubs.cpp
        ${HEADERS}
        )

# Link the specific LLVM libraries that WAMR needs
llvm_map_components_to_libnames(
        WAMR_LLVM_LIBRARIES
        core
        lto
        executionengine
        mcjit
)

# Link everything together
faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule
        wasm
        wamrlib
        )

if(FAASM_WAMR_AOT_MODE)
    target_link_libraries(wamrmodule
            aotclib
            ${WAMR_LLVM_LIBRARIES}
            )
endif()

