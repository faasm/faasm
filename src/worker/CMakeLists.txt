include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(worker_lib
        cgroup.cpp
        network.cpp
        pool.cpp
        ${FAASM_INCLUDE_DIR}/worker/worker.h
        )

target_link_libraries(worker_lib wasm)

add_executable(worker worker.cpp)
target_link_libraries(worker worker_lib)
