include_directories(${FAASM_INCLUDE_DIR}/edge)

add_library(edge SHARED STATIC edge.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(edge proto redis pistache pthread)

add_executable(edge_server server.cpp)

target_link_libraries(edge_server edge redis pistache pthread)