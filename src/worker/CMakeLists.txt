add_executable(worker worker.cpp)

target_link_libraries(worker redis proto util)