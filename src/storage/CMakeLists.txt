faasm_private_lib(storage
    FileDescriptor.cpp
    FileLoader.cpp
    FileSystem.cpp
    S3Wrapper.cpp
    SharedFiles.cpp
)
target_include_directories(storage PRIVATE ${FAASM_INCLUDE_DIR}/storage)
target_link_libraries(storage PUBLIC
    faasm::wavmmodule
    faasm::wamrmodule
    miniocpp::miniocpp
)
