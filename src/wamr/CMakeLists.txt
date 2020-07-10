include_directories(
        ${FAASM_INCLUDE_DIR}/wamr
        ${WAMR_INCLUDE_DIR}
)

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
        )

set(LIB_FILES
        WAMRWasmModule.cpp
        faasm.cpp
        native.cpp
        ${HEADERS}
        )

faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule wasm wamr)

# Temporary runner
add_executable(wamr_runner wamr_runner.cpp)
target_link_libraries(wamr_runner wamrmodule)

# ----------------------------------------------------
# WAMR CONTAINMENT ZONE
# ----------------------------------------------------

# NOTE: it is NOT SAFE to include the WAMR CMake files at a higher level as they
# modify top-level variables (e.g. using include_directories).
#
# Instead we must ensure WAMR CMake file inclusion is isolated at the end of this
# file, to make sure it doesn't mess up the rest of our build.
#
# See third-party/wamr/core/shared/platform/linux-sgx/shared_platform.cmake for an
# example of why this is dangerous.

# This variable causes WAMR to either include/ not include the SGX libc and libcxx.
if(FAASM_SGX_SUPPORT)
    set(BUILD_UNTRUST_PART 0)
else()
    set(BUILD_UNTRUST_PART 1)
endif()

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(wamr ${WAMR_RUNTIME_LIB_SOURCE})
