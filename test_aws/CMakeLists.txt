file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../test_utils
)

add_executable(
        tests_aws
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests_aws test_utils scheduler wasm aws)

add_test(NAME aws_tests COMMAND "tests_aws/tests")
