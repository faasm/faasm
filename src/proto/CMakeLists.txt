include(FindProtobuf)

# See the example in the gRPC repo here:
# https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/CMakeLists.txt

set(protobuf_MODULE_COMPATIBLE TRUE)

find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf  \
    ${PROTOBUF_LIBRARY} \
    ${PROTOBUF_PROTOC_LIBRARY} \
    ${PROTOBUF_PROTOC_EXECUTABLE} \
")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

include_directories(${PROTOBUF_INCLUDE_DIR})

set(PROTOC_EXE /usr/local/bin/protoc)
set(GRPC_PLUGIN /usr/local/bin/grpc_cpp_plugin)

set(OUTPUT_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.pb.h
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.pb.grpc.cc
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.pb.grpc.h
        )

add_custom_command(
        OUTPUT ${OUTPUT_FILES}
        COMMAND protoc
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${CMAKE_CURRENT_LIST_DIR}"
        --plugin=protoc-gen-grpc=${GRPC_PLUGIN}
        faasm.proto
        DEPENDS faasm.proto
        )

faasm_private_lib(proto ${OUTPUT_FILES})
target_link_libraries(proto ${Protobuf_LIBRARIES} gRPC::grpc++)