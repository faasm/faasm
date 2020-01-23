include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

set(LIB_FILES
        IRModuleRegistry.cpp
        WasmModule.cpp
        syscalls.cpp
        syscalls_chaining.cpp
        syscalls_dynlink.cpp
        syscalls_env.cpp
        syscalls_faasm.cpp
        syscalls_io.cpp
        syscalls_maths.cpp
        syscalls_memory.cpp
        syscalls_mpi.cpp
        syscalls_network.cpp
        syscalls_openmp.cpp
        syscalls_threads.cpp
        syscalls_unsupported.cpp
        syscalls_util.cpp
        ts_syscalls.cpp
        ${HEADERS}
        )

faasm_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm state scheduler storage libWAVM)

add_executable(codegen_shared_obj codegen_shared_obj.cpp)
target_link_libraries(codegen_shared_obj wasm)

add_executable(codegen_func codegen_func.cpp)
target_link_libraries(codegen_func wasm)

add_executable(capacity_experiment capacity_experiment.cpp)
target_link_libraries(capacity_experiment wasm)
