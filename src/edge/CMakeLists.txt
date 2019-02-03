include_directories(${FAASM_INCLUDE_DIR}/edge)

add_library(edge_lib STATIC FunctionEndpoint.cpp ${FAASM_INCLUDE_DIR}/edge/FunctionEndpoint.h)

target_link_libraries(edge_lib
        scheduler
        pistache
        )

add_executable(edge edge_server.cpp)
target_link_libraries(edge edge_lib)
