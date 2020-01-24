#//SET(FAASM_USER omp)

find_package(OpenMP REQUIRED)

function(omp_original_func exec_name source_path)
    add_executable(${exec_name} ${source_path})
    target_link_libraries(${exec_name} pthread)
endfunction(omp_original_func)

if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    omp_original_func(hello-mp hello-mp.cpp)
    omp_original_func(parallel-for parallel-for.cpp)
    omp_original_func(reduction-average reduction-average.cpp)
    omp_original_func(reduction-integral reduction-integral.cpp)
endif()


