include_directories(
        ${FAASM_INCLUDE_DIR}/wamr
)

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
        )

set(LIB_FILES
        WAMRWasmModule.cpp
        dynlink.cpp
        filesystem.cpp
        funcs.cpp
        native.cpp
        pthread.cpp
        state.cpp
        stubs.cpp
        ${HEADERS}
        )

faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule wasm wamr)

# Link wamr here at the end of the file
if(FAASM_SGX_SUPPORT)
    set(BUILD_UNTRUST_PART 0)
else()
    set(BUILD_UNTRUST_PART 1)
endif()

include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
add_library(wamr ${WAMR_RUNTIME_LIB_SOURCE})
