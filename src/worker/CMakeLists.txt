include_directories(${FAASM_INCLUDE_DIR}/worker)

add_library(worker STATIC worker.cpp ${FAASM_INCLUDE_DIR}/worker/worker.h)

target_link_libraries(worker proto redis proto wavme)

add_executable(worker_client client.cpp)

target_link_libraries(worker_client worker)
