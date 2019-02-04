include_directories(${FAASM_INCLUDE_DIR}/upload)

add_library(upload_lib
        STATIC
        UploadServer.cpp
        ${FAASM_INCLUDE_DIR}/upload/UploadServer.h
        )

target_link_libraries(upload_lib
        wasm
        proto
        ssl
        crypto
        cpprest
        )

add_executable(upload upload_server.cpp)
target_link_libraries(upload upload_lib)
