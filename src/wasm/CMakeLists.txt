include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

add_library(wasm STATIC
        CallChain.cpp
        FunctionLoader.cpp
        LocalFunctionLoader.cpp
        S3FunctionLoader.cpp
        instance.cpp
        WasmModule.cpp
        syscalls.cpp
        ${HEADERS}
        )

target_link_libraries(wasm scheduler memory state Logging IR WASTParse WASM Runtime)
