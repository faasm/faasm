add_executable(state_tpt_server state_tpt.h state_tpt_server.cpp)
target_link_libraries(state_tpt_server state)

add_executable(state_tpt_client state_tpt.h state_tpt_client.cpp)
target_link_libraries(state_tpt_client state)
