cmake_minimum_required(VERSION 3.0)
project(shared_experiment)

set(COMPILER_FLAGS "-nostdlib -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS " \
        -Xlinker --shared \
        -Xlinker --stack-first \
        -Xlinker --no-entry \
        -Xlinker --export-all \
    ")

add_library(lib_experiment SHARED lib_experiment.h lib_experiment.cpp)