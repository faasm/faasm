include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(wasm STATIC
        chaining.cpp
        wasm.cpp
        syscalls.cpp
        ${FAASM_INCLUDE_DIR}/wasm/wasm.h
        )

target_link_libraries(wasm infra Logging IR WASTParse WASM Runtime)
