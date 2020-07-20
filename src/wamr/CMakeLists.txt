# Set up WAMR itself
if(FAASM_SGX_SUPPORT)
    set(BUILD_UNTRUST_PART 0)
else()
    set(BUILD_UNTRUST_PART 1)
endif()

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
faasm_private_lib(wamr ${WAMR_RUNTIME_LIB_SOURCE})

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
