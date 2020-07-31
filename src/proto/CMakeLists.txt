include(FindProtobuf)

set(protobuf_MODULE_COMPATIBLE TRUE)

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

message(STATUS "Using protobuf ${protobuf_VERSION}")
message(STATUS "Using gRPC ${gRPC_VERSION}")

include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_grpc_cpp(PROTO_SRCS PROTO_HDRS state_service.proto)
protobuf_generate_grpc_cpp(PROTO_SRCS PROTO_HDRS function_service.proto)

set(LIB_FILES ${PROTO_SRCS} ${PROTO_HDRS})
message(-- "Protobuf_LIBRARIES=${Protobuf_LIBRARIES}")

faasm_private_lib(proto ${LIB_FILES})
target_link_libraries(proto ${Protobuf_LIBRARIES} gRPC::grpc++)