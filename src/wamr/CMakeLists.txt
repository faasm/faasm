# Unfortunately the pre-canned WAMR CMake file modifies too many top-level
# variables (e.g. through use of include_directories or changing global
# CFLAGS).
#
# We have our own custom CMake set-up which looks a bit like the wamrc build:
# https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/wamr-compiler/CMakeLists.txt

# Here we include all the components of WAMR we want
include(${WAMR_SHARED_DIR}/platform/${WAMR_BUILD_PLATFORM}/shared_platform.cmake)
include(${WAMR_SHARED_DIR}/mem-alloc/mem_alloc.cmake)
include(${WAMR_SHARED_DIR}/utils/shared_utils.cmake)
include(${WAMR_SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)
include(${WAMR_IWASM_DIR}/common/iwasm_common.cmake)
include(${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)

set(COMMON_WAMR_SRC
        ${PLATFORM_SHARED_SOURCE}
        ${MEM_ALLOC_SHARED_SOURCE}
        ${IWASM_COMMON_SOURCE}
        ${UTILS_SHARED_SOURCE}
        ${IWASM_INTERP_SOURCE}
        )

# SGX/ Non-SGX parts
if (FAASM_SGX_SUPPORT)
    set(BUILD_UNTRUST_PART 0)

    include(${WAMR_IWASM_DIR}/libraries/libc-builtin/libc_builtin.cmake)

    set(EXTRA_WAMR_SRC
            ${LIBC_BUILTIN_SOURCE}
            )

    set(EXTRA_LINK "")
else ()
    set(BUILD_UNTRUST_PART 1)

    # Turn on WAMR features
    add_definitions(-DWASM_ENABLE_BULK_MEMORY=1)
    add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)
    add_definitions(-DWASM_ENABLE_MULTI_MODULE=1)

    # Include top-level Linux config
    include(${WAMR_IWASM_DIR}/aot/iwasm_aot.cmake)

    include(${WAMR_IWASM_DIR}/libraries/libc-wasi/libc_wasi.cmake)
    include(${WAMR_IWASM_DIR}/libraries/lib-pthread/lib_pthread.cmake)
    include(${WAMR_IWASM_DIR}/libraries/thread-mgr/thread_mgr.cmake)

    set(EXTRA_WAMR_SRC
            ${IWASM_AOT_SOURCE}
            ${LIBC_WASI_SOURCE}
            ${LIB_PTHREAD_SOURCE}
            ${THREAD_MGR_SOURCE}
            )

    include(${WAMR_IWASM_DIR}/compilation/iwasm_compl.cmake)
    faasm_private_lib(aotclib "${IWASM_COMPL_SOURCE}")
    set(EXTRA_LINK aotclib)
endif ()

# Build the WAMR lib
set(ALL_WAMR_SRC ${COMMON_WAMR_SRC} ${EXTRA_WAMR_SRC})
faasm_private_lib(wamrlib "${ALL_WAMR_SRC}")

# -----------------------------
# Faasm-specific WAMR module
# -----------------------------

include_directories(
        ${FAASM_INCLUDE_DIR}/wamr
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
        native.cpp
        pthread.cpp
        state.cpp
        stubs.cpp
        ${HEADERS}
        )

# Link the specific LLVM libraries that WAMR needs
llvm_map_components_to_libnames(
        WAMR_LLVM_LIBRARIES
        lto
        executionengine
        mcjit
)

# Link everything together
faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule
        wasm
        ${EXTRA_LINK}
        wamrlib
        ${WAMR_LLVM_LIBRARIES}
        )
