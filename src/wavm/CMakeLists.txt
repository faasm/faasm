
faasm_private_lib(wavmmodule
    WAVMWasmModule.cpp
    WAVMModuleCache.cpp
    IRModuleCache.cpp
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
    timing.cpp
    util.cpp
)
target_include_directories(wavmmodule PRIVATE ${FAASM_INCLUDE_DIR}/wavm)
target_include_directories(wavmmodule PUBLIC ${FAASM_WAVM_SOURCE_DIR}/Lib)
target_link_libraries(wavmmodule PUBLIC
    faasm::wasm
    libWAVM
    faasm::threads
)
