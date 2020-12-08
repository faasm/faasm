set(FAASM_USER mpi)

set(OMPI_CXX ${CXX})
set(OMPI_C ${C})

function(mpi_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})

    target_link_libraries(${exec_name} faasmpi mpi)

    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Wasm")
        target_link_libraries(${exec_name} pthread)
    endif ()

    # Add to list of all funcs
    set(ALL_MPI_FUNCS ${ALL_MPI_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(mpi_func)

mpi_func(mpi_allgather mpi_allgather.cpp)
mpi_func(mpi_allreduce mpi_allreduce.cpp)
mpi_func(mpi_alltoall mpi_alltoall.cpp)
mpi_func(mpi_barrier mpi_barrier.cpp)
mpi_func(mpi_bcast mpi_bcast.cpp)
mpi_func(mpi_cartesian mpi_cartesian.cpp)
mpi_func(mpi_cart_create mpi_cart_create.cpp)
mpi_func(mpi_checks mpi_checks.cpp)
mpi_func(mpi_gather mpi_gather.cpp)
mpi_func(mpi_isendrecv mpi_isendrecv.cpp)
mpi_func(mpi_onesided mpi_onesided.cpp)
mpi_func(mpi_order mpi_order.cpp)
mpi_func(mpi_probe mpi_probe.cpp)
mpi_func(mpi_put mpi_put.cpp)
mpi_func(mpi_reduce mpi_reduce.cpp)
mpi_func(mpi_reduce_dbl mpi_reduce_dbl.cpp)
mpi_func(mpi_scan mpi_scan.cpp)
mpi_func(mpi_scatter mpi_scatter.cpp)
mpi_func(mpi_status mpi_status.cpp)
mpi_func(mpi_typesize mpi_typesize.cpp)
mpi_func(mpi_wincreate mpi_wincreate.cpp)

mpi_func(hellompi hellompi.cpp)

# Custom target for building all functions
add_custom_target(mpi_all_funcs DEPENDS ${ALL_MPI_FUNCS})
