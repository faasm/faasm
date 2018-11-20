add_library(catch INTERFACE)

file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

add_executable(
        tests
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(tests faasm proto catch edge_lib upload_lib worker_lib emulator)

add_test(NAME tests COMMAND "test/tests")
