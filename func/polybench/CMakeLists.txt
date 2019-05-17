set(FAASM_USER polybench)

include_directories(${CMAKE_CURRENT_LIST_DIR}/utilities)

function(polybench_func exec_name c_file h_file)
    add_executable(poly_${exec_name} ${c_file} ${h_file} utilities/polybench.c utilities/polybench.h)

    if (FAASM_BUILD_TYPE STREQUAL "emscripten")
        set(MALLOC_LIB "dlmalloc")
    else()
        set(MALLOC_LIB "")
    endif()

    target_link_libraries(poly_${exec_name} faasm ${MALLOC_LIB})

endfunction(polybench_func)

polybench_func(covariance datamining/covariance/covariance.c datamining/covariance/covariance.h)
