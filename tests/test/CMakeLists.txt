file(GLOB_RECURSE TEST_FILES test_*.cpp)
file(GLOB_RECURSE ENCLAVE_TEST_FILES enclave/test_*.cpp)
file(GLOB_RECURSE ATTESTATION_TEST_FILES attestation/test_*.cpp)

# Remove SGX tests if SGX disabled or not found
if (FAASM_SGX_MODE STREQUAL "Disabled")
    list(REMOVE_ITEM TEST_FILES ${ENCLAVE_TEST_FILES})
    list(REMOVE_ITEM TEST_FILES ${ATTESTATION_TEST_FILES})
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
