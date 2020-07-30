# Build of WAMR for SGX support

set(BUILD_UNTRUST_PART 0)

set(WAMR_BUILD_TARGET X86_64)
set(WAMR_BUILD_PLATFORM linux-sgx)
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_AOT 0)
set(WAMR_BUILD_LIBC_WASI 0)
set(WAMR_BUILD_LIBC_BUILTIN 1)

include(${FAASM_WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(wamrlib_sgx ${WAMR_RUNTIME_LIB_SOURCE})


## --------------------------------------------------------
## WAMR SGX SUPPORT
## --------------------------------------------------------
#
## Note, although BUILD_UNTRUST_PART is used in shared_platform.cmake, it uses the
## global include_dirs function, which we want to avoid, so we'll include SGX stdlibs
## manually
## set(BUILD_UNTRUST_PART 0)
#
## Here we include all the components of WAMR we want
#include(${WAMR_SHARED_DIR}/platform/linux-sgx/shared_platform.cmake)
#include(${WAMR_SHARED_DIR}/mem-alloc/mem_alloc.cmake)
#include(${WAMR_SHARED_DIR}/utils/shared_utils.cmake)
#include(${WAMR_SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)
#include(${WAMR_IWASM_DIR}/common/iwasm_common.cmake)
#include(${WAMR_IWASM_DIR}/interpreter/iwasm_interp.cmake)
#include(${WAMR_IWASM_DIR}/libraries/libc-builtin/libc_builtin.cmake)
#
#set(ALL_WAMR_SRC
#        ${PLATFORM_SHARED_SOURCE}
#        ${MEM_ALLOC_SHARED_SOURCE}
#        ${IWASM_COMMON_SOURCE}
#        ${UTILS_SHARED_SOURCE}
#        ${IWASM_INTERP_SOURCE}
#        ${LIBC_BUILTIN_SOURCE}
#        )
#
## Build the WAMR lib
#add_library(wamrlib_sgx "${ALL_WAMR_SRC}")
#
#target_include_directories(wamrlib_sgx PUBLIC
#        ${SGX_SDK_PATH}/include
#        ${SGX_SDK_DIR}/include/tlibc
#        ${SGX_SDK_DIR}/include/libcxx
#)
