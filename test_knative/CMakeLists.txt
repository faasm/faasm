file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../test_utils
)

add_executable(
        tests_knative
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests_knative
        test_utils
        emulator
        knative_native
        )

add_test(NAME knative_tests COMMAND "tests_knative/tests")
