faasm_private_lib(upload_lib
    UploadServer.cpp
)
target_include_directories(upload_lib PRIVATE ${FAASM_INCLUDE_DIR}/upload)
target_link_libraries(upload_lib PUBLIC
    faasm::codegen
    cpprestsdk::cpprestsdk
    faasm::faaslet_lib
    faasm::wavmmodule
    faasm::wamrmodule
    faasm::system
    faasm::sgx
)

add_executable(upload upload_server.cpp)
target_link_libraries(upload PUBLIC faasm::upload_lib)
