include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(wasm STATIC
        CallChain.cpp
        FunctionLoader.cpp
        WasmModule.cpp
        syscalls.cpp
        ${FAASM_INCLUDE_DIR}/wasm/CallChain.h
        ${FAASM_INCLUDE_DIR}/wasm/WasmModule.h
        )

target_link_libraries(wasm scheduler state Logging IR WASTParse WASM Runtime)
