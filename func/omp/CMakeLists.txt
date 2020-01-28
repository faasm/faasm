set(FAASM_USER omp)

if(FAASM_BUILD_TYPE STREQUAL "wasm")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
else()
    find_package(OpenMP REQUIRED)
    include_directories(${OPENMP_INCLUDE_DIR})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(OMPI_CXX ${CXX})
set(OMPI_C ${C})

function(omp_func exec_name dir_path link_mpi)
    faasm_func(${exec_name} ${dir_path})

    if(FAASM_BUILD_TYPE STREQUAL "wasm")
        set(MPI_LIB "")
    else()
        include_directories(/usr/local/faasm/openmpi/include)
        set(MPI_LIB /usr/local/faasm/openmpi/lib/libmpi.so)
    endif()

    if(FAASM_BUILD_TYPE STREQUAL "knative-native")
        set(EXEC_NAME ${exec_name}-knative)
    else()
        set(EXEC_NAME ${exec_name})
    endif()

    if (FAASM_BUILD_TYPE STREQUAL "wasm")
        target_link_libraries(${EXEC_NAME} faasmp)
    else ()
        target_link_libraries(${EXEC_NAME} pthread)
    endif ()

    if(link_mpi STREQUAL "true")
        target_link_libraries(${EXEC_NAME} ${MPI_LIB})
    endif()
endfunction(omp_func)

omp_func(checks checks.cpp false)
omp_func(hellomp hellomp.cpp false)

# Skip MPI stuff in knative for now
if(FAASM_BUILD_TYPE STREQUAL "knative-native")
else()
omp_func(hellompi hellompi.cpp true)
endif()
