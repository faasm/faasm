file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../utils
)

add_executable(
        faabric_tests
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(faabric_tests faabric_test_utils)

add_test(NAME faabric_tests COMMAND "tests/test/faabric_tests")

