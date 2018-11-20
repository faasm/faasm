include_directories(${FAASM_INCLUDE_DIR}/edge)

# Edge
add_library(edge_lib STATIC edge.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(edge_lib
        wasm
        proto
        infra
        pistache
        )

add_executable(edge edge_server.cpp)
target_link_libraries(edge edge_lib)

# Upload
add_library(upload_lib STATIC upload.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(upload_lib
        wasm
        proto
        infra
        ssl
        crypto
        cpprest
        )

add_executable(upload upload_server.cpp)
target_link_libraries(upload edge_lib)
