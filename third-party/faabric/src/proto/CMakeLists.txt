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
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.grpc.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/faasm.grpc.pb.h
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

include_directories(${FAASM_INCLUDE_DIR}/proto/)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/proto/*.h")

set(LIB_FILES
        RPCServer.cpp
        ${HEADERS}
        ${OUTPUT_FILES}
        )

if (FAASM_BUILD_TYPE STREQUAL "native-tools")
    faasm_public_lib(proto "${LIB_FILES}")
else ()
    faasm_private_lib(proto "${LIB_FILES}")
endif ()

target_link_libraries(proto PRIVATE
        ${Protobuf_LIBRARIES}
        gRPC::grpc++
        gRPC::grpc++_reflection
        )
