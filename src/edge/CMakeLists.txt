include_directories(${FAASM_INCLUDE_DIR}/edge)

add_library(edge_lib STATIC edge.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(edge_lib
        wasm
        proto
        infra
        ssl crypto
        cpprest
        )

add_executable(edge server.cpp)

target_link_libraries(edge edge_lib)