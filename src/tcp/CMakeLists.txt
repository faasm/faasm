include_directories(${FAASM_INCLUDE_DIR}/tcp)

set(HEADERS
        ${FAASM_INCLUDE_DIR}/tcp/EchoServer.h
        ${FAASM_INCLUDE_DIR}/tcp/TCPClient.h
        ${FAASM_INCLUDE_DIR}/tcp/TCPMessage.h
        ${FAASM_INCLUDE_DIR}/tcp/TCPServer.h
    )

set(LIB_FILES
        EchoServer.cpp
        TCPClient.cpp
        TCPMessage.cpp
        TCPServer.cpp
        ${HEADERS}
        )

faasm_private_lib(tcp "${LIB_FILES}")

target_link_libraries(tcp util)
