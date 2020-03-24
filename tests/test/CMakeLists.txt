file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

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
