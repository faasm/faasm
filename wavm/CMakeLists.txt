cmake_minimum_required(VERSION 3.10)
project(faasm)

set(CMAKE_CXX_STANDARD 11)

add_executable(faasm src/worker.cpp)

target_link_libraries(faasm cpp_redis tacopie pthread)
