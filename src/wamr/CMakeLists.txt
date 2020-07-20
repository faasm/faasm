# WAMR memory alloc
include (${WAMR_SHARED_DIR}/mem-alloc/mem_alloc.cmake)
set(WAMR_SRC ${WAMR_SRC} ${MEM_ALLOC_SHARED_SOURCE})

# Common iwasm
include (${WAMR_IWASM_DIR}/common/iwasm_common.cmake)
set(WAMR_SRC ${WAMR_SRC} ${IWASM_COMMON_SOURCE})

# Utils
include (${WAMR_SHARED_DIR}/utils/shared_utils.cmake)
set(WAMR_SRC ${WAMR_SRC} ${UTILS_SHARED_SOURCE})

# WAMR interpreter (must be included for _both_ JIT and interp)
include (${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)
set(WAMR_SRC ${WAMR_SRC} ${IWASM_INTERP_SOURCE})

# SGX/ Non-SGX parts
if(FAASM_SGX_SUPPORT)
    set(BUILD_UNTRUST_PART 0)
    include (${WAMR_SHARED_DIR}/platform/linux-sgx/shared_platform.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${PLATFORM_SHARED_SOURCE})

    # Builtin libc
    include (${WAMR_IWASM_DIR}/libraries/libc-builtin/libc_builtin.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${LIBC_BUILTIN_SOURCE})

else()
    # Turn on WAMR features
    add_definitions(-DWASM_ENABLE_BULK_MEMORY=1)
    add_definitions(-DWASM_ENABLE_WAMR_COMPILER=1)
    add_definitions(-DWASM_ENABLE_MULTI_MODULE=1)

    # Include top-level Linux config
    set(BUILD_UNTRUST_PART 1)
    include (${WAMR_SHARED_DIR}/platform/linux/shared_platform.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${PLATFORM_SHARED_SOURCE})

    # WAMR compiler
    include (${WAMR_IWASM_DIR}/aot/iwasm_aot.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${IWASM_AOT_SOURCE})

    include (${WAMR_IWASM_DIR}/compilation/iwasm_compl.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${IWASM_COMPL_SOURCE})

    # WASI libc
    include (${WAMR_IWASM_DIR}/libraries/libc-wasi/libc_wasi.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${LIBC_WASI_SOURCE})

    # WASI pthreads
    include (${WAMR_IWASM_DIR}/libraries/lib-pthread/lib_pthread.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${LIB_PTHREAD_SOURCE})

    include (${WAMR_IWASM_DIR}/libraries/thread-mgr/thread_mgr.cmake)
    set(WAMR_SRC ${WAMR_SRC} ${THREAD_MGR_SOURCE})
endif()

faasm_private_lib(wamr "${WAMR_SRC}")

# Link WAMR with LLVM for codegen
target_link_libraries(wamr ${LLVM_AVAILABLE_LIBS})

# Set up our own WAMR module
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

# Build our wamr module
faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule wasm wamr)
