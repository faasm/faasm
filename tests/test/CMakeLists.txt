file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)
file(GLOB_RECURSE SGX_TEST_FILES sgx test_*.cpp)

message(STATUS ${TEST_FILES})

# Remove SGX tests if SGX disabled or not found
if (NOT (FAASM_SGX_ENABLED AND SGX_FOUND))
    list(REMOVE_ITEM ${TEST_FILES} ${SGX_TEST_FILES})
endif()

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../utils
)

add_executable(
        tests
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests test_utils)

add_test(NAME tests COMMAND "tests/test/tests")

