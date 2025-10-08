
faasm_private_lib(test_utils
    faasm_fixtures.cpp
    worker_utils.cpp
    utils.h
)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_utils PUBLIC
    faasm::wasm_codegen
    faasm::enclave
    faasm::runner_lib
    faasm::faaslet_lib
    Catch2::Catch2
    faabric::test_utils
)

# Catch2 signal handler interferes with faasm and sanitiser signal handlers
# Disabling it makes logs easier to understand
target_compile_definitions(test_utils PUBLIC
    CATCH_CONFIG_NO_POSIX_SIGNALS=1
)
