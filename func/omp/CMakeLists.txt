set(FAASM_USER omp)

set(CMAKE_C_FLAGS "-fopenmp ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")

set(OMPI_CXX ${CXX})
set(OMPI_C ${C})

function(omp_func exec_name dir_path)
    add_executable(${exec_name} ${dir_path})

    if (FAASM_BUILD_TYPE STREQUAL "wasm")
        target_link_libraries(${exec_name} faasmp faasm)
    else ()
        target_link_libraries(${exec_name} pthread faasm emulator mpi)
    endif ()
endfunction(omp_func)

omp_func(hellomp hellomp.cpp)
omp_func(hellompi hellompi.cpp)
