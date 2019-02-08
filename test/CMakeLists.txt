file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(
        tests
        main.cpp
        utils/matrix_utils.cpp
        utils/message_utils.cpp
        utils/system_utils.cpp
        utils.h
        ${TEST_FILES}
)

target_link_libraries(tests catch faasm edge_lib upload_lib worker_lib emulator data)

add_test(NAME tests COMMAND "test/tests")
