file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR} test_*.cpp)
file(GLOB_RECURSE SGX_TEST_FILES sgx test_*.cpp)

# Remove SGX tests if SGX disabled or not found
if (NOT SGX_FOUND)
    list(REMOVE_ITEM ${TEST_FILES} ${SGX_TEST_FILES})
endif()

add_executable(
    tests
    main.cpp
    ${TEST_FILES}
)

target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tests PRIVATE faasm::test_utils)

add_test(NAME tests COMMAND "tests/test/tests")
