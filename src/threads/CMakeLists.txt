file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/threads/*.h")

set(LIB_FILES
        MutexManager.cpp
        ThreadState.cpp
        ${HEADERS}
        )

faasm_private_lib(threads "${LIB_FILES}")
target_link_libraries(threads util)
