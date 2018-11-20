include_directories(${FAASM_INCLUDE_DIR}/edge)

# Edge
add_library(edge_lib STATIC edge.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(edge_lib
        proto
        infra
        pistache
        )

add_executable(edge edge_server.cpp)
target_link_libraries(edge edge_lib)
