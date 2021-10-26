
faasm_private_lib(test_utils
    system_utils.cpp
    worker_utils.cpp
    utils.h
)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(test_utils PUBLIC
    faasm::codegen
    faasm::upload_lib
    faasm::runner_lib
    cpprestsdk::cpprestsdk
    faasm::faaslet_lib
    faasm::wavmmodule
    faasm::wamrmodule
    faasm::system
    faasm::sgx
    Catch2::Catch2
    faabric::test_utils
)
