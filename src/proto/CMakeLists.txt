include_directories(${PROTOBUF_INCLUDE_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS faasm.proto)

add_library(proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(proto ${Protobuf_LIBRARIES})