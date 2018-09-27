include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

add_executable(worker
        cgroup.cpp
        network.cpp
        worker.cpp
        threads.cpp
        ${FAASM_INCLUDE_DIR}/worker/worker.h
    )

target_link_libraries(worker wasm)
