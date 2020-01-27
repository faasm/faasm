set(FAASM_USER omp)

set(CMAKE_C_FLAGS "-fopenmp ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")

set(OMPI_CXX ${CXX})
set(OMPI_C ${C})

function(omp_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})

    if (FAASM_BUILD_TYPE STREQUAL "wasm")
        target_link_libraries(${exec_name} faasmp)
    else ()
        set(MPI_LIB /usr/local/faasm/openmpi/libopenmpi)

        target_link_libraries(${exec_name} pthread)
    endif ()
endfunction(omp_func)

omp_func(hellomp hellomp.cpp)
omp_func(hellompi hellompi.cpp)
