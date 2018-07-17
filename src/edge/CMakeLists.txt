add_executable(edge edge.cpp)

target_link_libraries(edge proto redis pistache pthread)