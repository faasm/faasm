include_directories(
        ${FAASM_INCLUDE_DIR}/wasm
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/*.h")

set(LIB_FILES
        WasmEnvironment.cpp
        WasmModule.cpp
        chaining_util.cpp
        wamr/WAMRWasmModule.cpp
        wavm/WAVMWasmModule.cpp
        wavm/syscalls.h
        wavm/chaining.cpp
        wavm/dynlink.cpp
        wavm/env.cpp
        wavm/faasm.cpp
        wavm/io.cpp
        wavm/libcxx.cpp
        wavm/maths.cpp
        wavm/memory.cpp
        wavm/messages.cpp
        wavm/mpi.cpp
        wavm/network.cpp
        wavm/openmp.cpp
        wavm/process.cpp
        wavm/rust.cpp
        wavm/scheduling.cpp
        wavm/signals.cpp
        wavm/syscalls.cpp
        wavm/threads.cpp
        wavm/timing.cpp
        wavm/typescript.cpp
        wavm/util.cpp
        ${HEADERS}
        )

faasm_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm ir_cache mpi state scheduler storage libWAVM)
