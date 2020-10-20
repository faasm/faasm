include_directories(
        ${FAASM_INCLUDE_DIR}/wavm
        ${FAASM_WAVM_SOURCE_DIR}/Lib
)

set(HEADERS
        "${FAASM_INCLUDE_DIR}/wavm/OMPThreadPool.h"
        "${FAASM_INCLUDE_DIR}/wavm/WAVMWasmModule.h"
        )

add_subdirectory(openmp)

set(LIB_FILES
        WAVMWasmModule.cpp
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
        OMPThreadPool.cpp
        process.cpp
        rust.cpp
        scheduling.cpp
        signals.cpp
        syscalls.cpp
        threads.cpp
        timing.cpp
        typescript.cpp
        util.cpp
        ${HEADERS}
        )

faasm_private_lib(wavmmodule "${LIB_FILES}")
target_link_libraries(wavmmodule wasm ir_cache libWAVM)
