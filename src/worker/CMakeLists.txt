include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(worker_lib STATIC
        wasm.cpp
        intrinsic.cpp
        cgroup.cpp
        resolver.h
        ${FAASM_INCLUDE_DIR}/worker/intrinsic.h
        ${FAASM_INCLUDE_DIR}/worker/worker.h
    )

target_link_libraries(worker_lib infra Logging IR WASTParse WASM Runtime Emscripten ThreadTest)

add_executable(worker worker.cpp)
target_link_libraries(worker worker_lib)
