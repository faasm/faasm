faasm_private_lib(wasm
    WasmEnvironment.cpp
    WasmExecutionContext.cpp
    WasmModule.cpp
    chaining_util.cpp
)

# Populate the variables defined for cross-compiling WASM
set(FAASM_WASM_BYTES_PER_PAGE $ENV{FAASM_WASM_BYTES_PER_PAGE})
set(FAASM_WASM_CTORS_FUNC_NAME $ENV{FAASM_WASM_CTORS_FUNC_NAME})
set(FAASM_WASM_MAX_MEMORY $ENV{FAASM_WASM_MAX_MEMORY})
set(FAASM_WASM_STACK_SIZE $ENV{FAASM_WASM_STACK_SIZE})
set(FAASM_WASM_ZYGOTE_FUNC_NAME $ENV{FAASM_WASM_ZYGOTE_FUNC_NAME})
configure_file(
    ${FAASM_INCLUDE_DIR}/wasm/WasmCommon.h.in
    ${FAASM_GENERATED_INCLUDE_DIR}/wasm/WasmCommon.h
)
target_include_directories(wasm PRIVATE ${FAASM_INCLUDE_DIR}/wasm)

target_link_libraries(wasm PUBLIC
    faasm::conf
    faasm::storage
    faasm::threads
)
