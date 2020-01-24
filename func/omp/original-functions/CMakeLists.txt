SET(FAASM_USER omp)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

function(omp_original_func exec_name source_path)
    add_executable(${exec_name} ${source_path})
    target_link_libraries(${exec_name} pthread)
endfunction(omp_original_func)

omp_original_func(hello-mp hello-mp.cpp)
omp_original_func(parallel-for parallel-for.cpp)
omp_original_func(reduction-average reduction-average.cpp)
omp_original_func(reduction-integral reduction-integral.cpp)
