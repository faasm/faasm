include_directories(${FAASM_INCLUDE_DIR}/worker ${FAASM_WAVM_SOURCE_DIR})

add_library(worker STATIC worker.cpp wasm.cpp ${FAASM_INCLUDE_DIR}/worker/worker.h)
target_link_libraries(worker IR WAST WASM Runtime Emscripten ThreadTest Logging infra)

add_executable(worker_client client.cpp)
target_link_libraries(worker_client worker)

add_executable(worker_dummy dummy.cpp)
target_link_libraries(worker_dummy worker)
