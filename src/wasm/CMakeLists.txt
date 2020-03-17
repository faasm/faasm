include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

set(LIB_FILES
        WasmModule.cpp
        chaining_util.cpp
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
target_link_libraries(wasm ir_cache mpi state scheduler storage libWAVM)
