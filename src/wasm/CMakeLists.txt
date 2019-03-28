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
        syscalls_dynlink.cpp
        syscalls_env.cpp
        syscalls_faasm.cpp
        syscalls_io.cpp
        syscalls_memory.cpp
        syscalls_network.cpp
        syscalls_unsupported.cpp
        syscalls_util.cpp
        ${HEADERS}
        )

target_link_libraries(wasm scheduler memory state WAVM)

add_executable(codegen codegen.cpp)
target_link_libraries(codegen wasm)
