# --------------------------------------------------------
# WAMR SGX-SPECIFIC BUILD
# --------------------------------------------------------
# Unfortunately it isn't possible to build both the non-SGX WAMR
# and SGX WAMR in the same project. This means we have to have this
# separate WAMR build to add SGX support.

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
