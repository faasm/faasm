faasm_private_lib(wasm
    WasmEnvironment.cpp
    WasmExecutionContext.cpp
    WasmModule.cpp
    chaining_util.cpp
    faasm.cpp
    host_interface_test.cpp
    migration.cpp
    openmp.cpp
    s3.cpp
    threads.cpp
)

# Shared variables with the cross-compilation toolchain
set(FAASM_WASM_BYTES_PER_PAGE CACHE STRING "Number of bytes per WASM page")
set(FAASM_WASM_CTORS_FUNC_NAME CACHE STRING "Name for the WASM constructors function")
set(FAASM_WASM_MAX_MEMORY CACHE STRING "Maximum memory for a WASM module instance (in Bytes)")
set(FAASM_WASM_STACK_SIZE CACHE STRING "Stack size for a WASM module instance (in Bytes)")
set(FAASM_WASM_ZYGOTE_FUNC_NAME CACHE STRING "Name for Faasm's Zygote function")

# Check that the required Cache variables are defined
if ((NOT FAASM_WASM_BYTES_PER_PAGE) OR
    (NOT FAASM_WASM_CTORS_FUNC_NAME) OR
    (NOT FAASM_WASM_MAX_MEMORY) OR
    (NOT FAASM_WASM_STACK_SIZE) OR
    (NOT FAASM_WASM_ZYGOTE_FUNC_NAME))
message(FATAL_ERROR "Some of the FAASM_WASM variables are not set!")
endif()

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
