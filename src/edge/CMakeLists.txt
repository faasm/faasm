add_executable(edge edge.cpp)

target_link_libraries(edge redis cpp_redis tacopie pistache pthread)
set_target_properties(edge PROPERTIES FOLDER edge)