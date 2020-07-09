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

if(${FAASM_SGX_SUPPORT})
    set(BUILD_UNTRUST_PART 0) #Have to define BUILD_UNTRUST_PART because then tlibc and tcxx will be included (see third-party/wamr/core/shared/platform/linux-sgx/shared_platform.cmake)
endif()
include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(wamr ${WAMR_RUNTIME_LIB_SOURCE})