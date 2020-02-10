set(FAASM_USER mpi)

set(OMPI_CXX ${CXX})
set(OMPI_C ${C})

function(mpi_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})

    if(FAASM_BUILD_TYPE STREQUAL "wasm")
        set(MPI_LIB "")
    else()
        include_directories(/usr/local/faasm/openmpi/include)
        set(MPI_LIB /usr/local/faasm/openmpi/lib/libmpi.so)
    endif()

    if (FAASM_BUILD_TYPE STREQUAL "wasm")
        target_link_libraries(${exec_name} faasmpi)
    else ()
        target_link_libraries(${exec_name} pthread)
    endif ()

    target_link_libraries(${exec_name} ${MPI_LIB})
endfunction(mpi_func)

# Skip MPI stuff in knative for now
if(FAASM_BUILD_TYPE STREQUAL "knative-native")
else()
mpi_func(mpi_barrier mpi_barrier.cpp)
mpi_func(mpi_bcast mpi_bcast.cpp)
mpi_func(mpi_checks mpi_checks.cpp)
mpi_func(mpi_order mpi_order.cpp)
mpi_func(mpi_probe mpi_probe.cpp)
mpi_func(mpi_scatter mpi_scatter.cpp)
mpi_func(mpi_gather mpi_gather.cpp)
mpi_func(mpi_status mpi_status.cpp)
mpi_func(hellompi hellompi.cpp)
endif()
