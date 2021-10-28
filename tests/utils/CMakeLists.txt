
faasm_private_lib(test_utils
    system_utils.cpp
    worker_utils.cpp
    utils.h
)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_utils PUBLIC
    faasm::codegen
    faasm::runner_lib
    faasm::faaslet_lib
    faasm::sgx
    Catch2::Catch2
    faabric::test_utils
)
