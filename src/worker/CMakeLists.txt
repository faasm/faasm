include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/worker/*.h")

add_library(worker_lib
        CGroup.cpp
        NetworkNamespace.cpp
        StateThread.cpp
        WorkerThreadPool.cpp
        ${HEADERS}
        )

target_link_libraries(worker_lib wasm scheduler)

add_executable(worker worker_main.cpp)
target_link_libraries(worker worker_lib)
