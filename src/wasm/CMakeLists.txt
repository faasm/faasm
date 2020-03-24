include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

set(LIB_FILES
        WasmModule.cpp
        chaining_util.cpp
        chaining.cpp
        dynlink.cpp
        env.cpp
        faasm.cpp
        io.cpp
        maths.cpp
        memory.cpp
        mpi.cpp
        network.cpp
        openmp.cpp
        rust_interface.cpp
        syscalls.cpp
        threads.cpp
        typescript.cpp
        unsupported.cpp
        util.cpp
        ${HEADERS}
        )

faasm_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm ir_cache mpi state scheduler storage libWAVM)
