include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}/Lib
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/storage/*.h")

set(LIB_FILES
        FileDescriptor.cpp
        FileLoader.cpp
        FileSystem.cpp
        FileserverFileLoader.cpp
        LocalFileLoader.cpp
        S3FileLoader.cpp
        S3Wrapper.cpp
        SharedFiles.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
add_dependencies(storage aws_ext)

find_package(aws-cpp-sdk-core)
find_package(aws-cpp-sdk-s3)

# This is necessary to make sure we can link the S3 module with the core one
add_dependencies(aws-cpp-sdk-s3 aws-cpp-sdk-core)

target_link_libraries(storage
    wavmmodule
    wamrmodule
    cpprestsdk::cpprest
    aws-cpp-sdk-core
    aws-cpp-sdk-s3
    curl
)
