file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../test_utils
)

add_executable(
        tests_ibm
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests_ibm test_utils scheduler wasm)

add_test(NAME integration_tests COMMAND "tests_ibm/tests")
