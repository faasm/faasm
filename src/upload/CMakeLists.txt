include_directories(${FAASM_INCLUDE_DIR}/upload)

add_library(upload_lib STATIC upload.cpp ${FAASM_INCLUDE_DIR}/upload/upload.h)

target_link_libraries(upload_lib
        wasm
        proto
        infra
        ssl crypto
        cpprest
        )

add_executable(upload upload_server.cpp)
target_link_libraries(upload upload_lib)
