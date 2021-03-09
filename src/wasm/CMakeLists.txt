include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
)

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wasm/chaining.h"
        "${FAASM_INCLUDE_DIR}/wasm/serialisation.h"
        "${FAASM_INCLUDE_DIR}/wasm/WasmEnvironment.h"
        "${FAASM_INCLUDE_DIR}/wasm/WasmModule.h"
        )

set(LIB_FILES
        WasmEnvironment.cpp
        WasmModule.cpp
        chaining_util.cpp
        ${HEADERS}
        )

faasm_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm storage)
