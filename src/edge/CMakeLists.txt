include_directories(${FAASM_INCLUDE_DIR}/edge)

add_library(edge_lib STATIC edge.cpp ${FAASM_INCLUDE_DIR}/edge/edge.h)

target_link_libraries(edge_lib
        wasm
        proto
        infra
        ssl crypto
        cpprest
        )

# Executable for adding functions offline
add_executable(offline offline.cpp)
target_link_libraries(offline edge_lib)

# Executable for running the server
add_executable(edge server.cpp)
target_link_libraries(edge edge_lib)
