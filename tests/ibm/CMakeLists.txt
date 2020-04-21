file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../utils
)

add_executable(
        tests_ibm
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests_ibm
        test_utils
        scheduler
        worker_ibm_lib
        )

add_test(NAME ibm_tests COMMAND "tests_ibm/tests")
