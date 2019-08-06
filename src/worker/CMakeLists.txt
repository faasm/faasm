include_directories(
        ${FAASM_INCLUDE_DIR}/worker
)

set(LIB_HEADERS
        ${FAASM_INCLUDE_DIR}/worker/WorkerThread.h
        ${FAASM_INCLUDE_DIR}/worker/WorkerThreadPool.h
        )

set(LIB_FILES
        WorkerThread.cpp
        WorkerThreadPool.cpp
        ${LIB_HEADERS}
        )

add_library(worker_lib "${LIB_FILES}")
target_link_libraries(worker_lib wasm scheduler system)

add_executable(worker worker_main.cpp)
target_link_libraries(worker worker_lib)
