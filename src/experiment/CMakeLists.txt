add_executable(max_mem_experiment max_mem_experiment.cpp)

add_executable(max_thread_experiment max_thread_experiment.cpp)
target_link_libraries(max_thread_experiment pthread)

add_executable(rlimit_experiment rlimit_experiment.cpp)

add_executable(tcp_server tcp_server.cpp)
target_link_libraries(tcp_server util)

add_executable(tcp_client tcp_client.cpp)
target_link_libraries(tcp_client util)
