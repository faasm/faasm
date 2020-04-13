include_directories(
        ${FAASM_INCLUDE_DIR}/wamr
        ${WAMR_INCLUDE_DIR}
)

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wamr/WAMRWasmModule.h"
        )

set(LIB_FILES
        WAMRWasmModule.cpp
        ${HEADERS}
        )

faasm_private_lib(wamrmodule "${LIB_FILES}")
target_link_libraries(wamrmodule wasm libwamr)

# Temporary runner
add_executable(wamr_runner wamr_runner.cpp)
target_link_libraries(wamr_runner wamrmodule)
