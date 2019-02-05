file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_LIST_DIR} test_*.cpp)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(
        integration_tests
        main.cpp
        ${TEST_FILES}
)

target_link_libraries(integration_tests scheduler wasm aws)

add_test(NAME integration_tests COMMAND "integration_test/tests")
