# Add the different test directories. Each subdirectory will populate the
# TEST_FILES variable, adding the source files for the tests
add_subdirectory(codegen)
add_subdirectory(conf)
add_subdirectory(faaslet)
add_subdirectory(ported_libs)
add_subdirectory(runner)
add_subdirectory(storage)
add_subdirectory(system)
add_subdirectory(threads)
add_subdirectory(upload)
add_subdirectory(wamr)
add_subdirectory(wasm)
add_subdirectory(wavm)

# Add SGX tests if enabled
if (NOT FAASM_SGX_MODE STREQUAL "Disabled")
    add_subdirectory(attestation)
    add_subdirectory(enclave)
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
