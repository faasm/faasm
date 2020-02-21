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

function(omp_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})

    if(FAASM_BUILD_TYPE STREQUAL "knative-native")
        set(EXEC_NAME ${exec_name}-knative)
    else()
        set(EXEC_NAME ${exec_name})
    endif()

    if (FAASM_BUILD_TYPE STREQUAL "wasm")
        target_link_libraries(${exec_name} faasmp)
    else ()
        target_link_libraries(${exec_name} pthread)
    endif ()

    # Add to list of all funcs
    set(ALL_OMP_FUNCS ${ALL_OMP_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(omp_func)

omp_func(custom_reduce custom_reduce.cpp)
omp_func(for_static_schedule for_static_schedule.cpp)
omp_func(header_api_support header_api_support.cpp)
omp_func(hellomp hellomp.cpp)
omp_func(omp_checks omp_checks.cpp)
omp_func(pi_calculation pi_calculation.cpp)
omp_func(reduction_average reduction_average.cpp)
omp_func(reduction_integral reduction_integral.cpp)
omp_func(simple_barrier simple_barrier.cpp)
omp_func(simple_critical simple_critical.cpp)
omp_func(simple_flush simple_flush.cpp)
omp_func(simple_for simple_for.cpp)
omp_func(simple_master simple_master.cpp)
omp_func(simple_reduce simple_reduce.cpp)

# Intel OMP files
omp_func(intel_nstreams intel_nstreams.cpp)
omp_func(intel_synch_p2p intel_synch_p2p.cpp)
omp_func(intel_transpose intel_transpose.cpp)

if(FAASM_BUILD_TYPE STREQUAL "wasm")
    omp_func(stack_debug stack_debug.cpp)
endif()

# Custom target for building all functions
add_custom_target(omp_all_funcs DEPENDS ${ALL_OMP_FUNCS})
