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

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

set(LIB_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/faasm_func.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/faasm_func.pb.h
        ${CMAKE_CURRENT_BINARY_DIR}/faasm_state.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/faasm_state.pb.h
    )

add_custom_command(
        OUTPUT ${LIB_FILES}
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${CMAKE_CURRENT_LIST_DIR}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${CMAKE_CURRENT_LIST_DIR}"
        )

faasm_private_lib(proto ${LIB_FILES})
target_link_libraries(proto ${Protobuf_LIBRARIES} gRPC::grpc++)