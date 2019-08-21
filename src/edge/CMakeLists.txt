include_directories(${FAASM_INCLUDE_DIR}/edge)

set(LIB_FILES
        FunctionEndpoint.cpp
        ${FAASM_INCLUDE_DIR}/edge/FunctionEndpoint.h
        )

faasm_private_lib(edge_lib "${LIB_FILES}")

target_link_libraries(edge_lib
        scheduler
        endpoint
        )

add_executable(edge edge_server.cpp)
target_link_libraries(edge edge_lib)
