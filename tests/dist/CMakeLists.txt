file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../utils
)

# Tests
add_executable(
    dist_tests
    fixtures.h
    main.cpp
    ${TEST_FILES}
)

target_link_libraries(dist_tests test_utils)

add_test(NAME dist_tests COMMAND "tests/dist/dist_tests")

# Server
add_executable(
    dist_test_server
    server.cpp
)

target_link_libraries(dist_test_server test_utils)

