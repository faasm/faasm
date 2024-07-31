set(TEST_FILES ${TEST_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/test_cloning.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_dynamic_modules.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_execution_context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_io.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_memory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_openmp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_snapshots.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_wasm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_wasm_s3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/test_wasm_state.cpp
    PARENT_SCOPE
)
