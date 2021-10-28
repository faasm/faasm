
faasm_private_lib(threads
    MutexManager.cpp
    ThreadState.cpp
)
target_include_directories(threads PRIVATE ${FAASM_INCLUDE_DIR}/threads)
