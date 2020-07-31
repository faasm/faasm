include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS faasm.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS state_service.proto)

set(LIB_FILES ${PROTO_SRCS} ${PROTO_HDRS})
message(-- "Protobuf_LIBRARIES=${Protobuf_LIBRARIES}")

faasm_private_lib(proto ${LIB_FILES})
target_link_libraries(proto ${Protobuf_LIBRARIES})