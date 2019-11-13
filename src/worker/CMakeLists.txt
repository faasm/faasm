include_directories(
        ${FAASM_INCLUDE_DIR}/worker
)

set(LIB_HEADERS
        ${FAASM_INCLUDE_DIR}/worker/worker.h
        ${FAASM_INCLUDE_DIR}/worker/WorkerMain.h
        ${FAASM_INCLUDE_DIR}/worker/WorkerThread.h
        ${FAASM_INCLUDE_DIR}/worker/WorkerThreadPool.h
        )

set(LIB_FILES
        worker.cpp
        WorkerMain.cpp
        WorkerThread.cpp
        WorkerThreadPool.cpp
        ${LIB_HEADERS}
        )

faasm_private_lib(worker_lib "${LIB_FILES}")
target_link_libraries(worker_lib wasm zygote system)

add_executable(worker worker_main.cpp)
target_link_libraries(worker worker_lib)
