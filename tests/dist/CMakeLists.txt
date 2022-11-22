# Add the different test directories. Each subdirectory will populate the
# TEST_FILES variable, adding the source files for the tests
add_subdirectory(chaining)
add_subdirectory(mpi)
add_subdirectory(state)
add_subdirectory(threads)

# Tests
add_executable(
    dist_tests
    fixtures.h
    main.cpp
    ${TEST_FILES}
)

target_include_directories(
    dist_tests
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(dist_tests PRIVATE faasm::test_utils)

add_test(NAME dist_tests COMMAND "tests/dist/dist_tests")

# Server
add_executable(
    dist_test_server
    server.cpp
)

target_include_directories(
    dist_test_server
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(dist_test_server PRIVATE faasm::runner_lib)

