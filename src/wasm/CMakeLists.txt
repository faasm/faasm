
faasm_private_lib(wasm
    WasmEnvironment.cpp
    WasmExecutionContext.cpp
    WasmModule.cpp
    chaining_util.cpp
)
target_include_directories(wasm PRIVATE ${FAASM_INCLUDE_DIR}/wasm)
target_link_libraries(wasm PUBLIC
    faasm::conf
    faasm::storage
    faasm::threads
)
