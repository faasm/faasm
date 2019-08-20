
add_executable(http_experiment http_experiment.cpp)

target_link_libraries(http_experiment pistache endpoint pthread util)

add_executable(mem_experiment mem_experiment.cpp)

add_executable(cow_experiment cow_experiment.cpp)
