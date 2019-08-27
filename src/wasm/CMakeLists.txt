include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

set(LIB_FILES
        IRModuleRegistry.cpp
        codegen.cpp
        WasmModule.cpp
        syscalls.cpp
        syscalls_chaining.cpp
        syscalls_dynlink.cpp
        syscalls_env.cpp
        syscalls_io.cpp
        syscalls_maths.cpp
        syscalls_memory.cpp
        syscalls_network.cpp
        syscalls_threads.cpp
        syscalls_unsupported.cpp
        syscalls_util.cpp
        ${HEADERS}
        )

faasm_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm scheduler memory state storage WAVM)

add_executable(codegen codegen_runner.cpp)
target_link_libraries(codegen wasm)

add_executable(capacity_experiment capacity_experiment.cpp)
target_link_libraries(capacity_experiment wasm)
