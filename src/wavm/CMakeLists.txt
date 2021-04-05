include_directories(
        ${FAASM_INCLUDE_DIR}/wavm
        ${FAASM_WAVM_SOURCE_DIR}/Lib
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wavm/*.h")

set(LIB_FILES
        WAVMWasmModule.cpp
        LoadedDynamicModule.cpp
        syscalls.h
        chaining.cpp
        codegen.cpp
        dynlink.cpp
        env.cpp
        faasm.cpp
        io.cpp
        libcxx.cpp
        maths.cpp
        memory.cpp
        messages.cpp
        mpi.cpp
        network.cpp
        openmp.cpp
        process.cpp
        scheduling.cpp
        signals.cpp
        syscalls.cpp
        threads.cpp
        ThreadState.cpp
        timing.cpp
        util.cpp
        ${HEADERS}
        )

faasm_private_lib(wavmmodule "${LIB_FILES}")
target_link_libraries(wavmmodule wasm ir_cache libWAVM)
