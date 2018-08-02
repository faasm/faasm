include_directories(${FAASM_INCLUDE_DIR}/worker ${FAASM_WAVM_SOURCE_DIR})

add_library(worker_lib STATIC wasm.cpp resolver.h ${FAASM_INCLUDE_DIR}/worker/worker.h)
target_link_libraries(worker_lib IR WAST WASM Runtime Emscripten ThreadTest Logging infra)

add_executable(worker worker.cpp)
target_link_libraries(worker worker_lib)

add_executable(worker_dummy dummy.cpp)
target_link_libraries(worker_dummy worker_lib)
