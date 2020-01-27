cmake_minimum_required(VERSION 3.9)

# Copies the toolchain config
# This is only OK if this will not change
set(INSTALL_DIR /usr/bin)
set(CMAKE_C_COMPILER ${INSTALL_DIR}/clang)
set(CMAKE_CXX_COMPILER ${INSTALL_DIR}/clang++)

find_package(OpenMP REQUIRED)

function(omp_original_func exec_name source_path)
    add_executable(${exec_name} ${source_path})
    target_link_libraries(${exec_name} pthread)
endfunction(omp_original_func)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    omp_original_func(hello-mp hello-mp.cpp)
    omp_original_func(create-parallel create-parallel.cpp)
    omp_original_func(parallel-for parallel-for.cpp)
    omp_original_func(reduction-average reduction-average.cpp)
    omp_original_func(reduction-integral reduction-integral.cpp)
    omp_original_func(create-teams create-teams.cpp)
    omp_original_func(intel-nstream intel-nstream.c)
    omp_original_func(intel-transpose intel-transpose.c)
endif()


