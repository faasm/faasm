set(FAASM_USER polybench)

include_directories(${CMAKE_CURRENT_LIST_DIR}/utilities)

function(poly_func exec_name file_name)
    add_executable(${exec_name} ${file_name} utilities/polybench.c utilities/polybench.h)

    if (FAASM_BUILD_TYPE STREQUAL "emscripten")
        set(MALLOC_LIB "dlmalloc")
    else()
        set(MALLOC_LIB "")
    endif()

    target_link_libraries(${exec_name} faasm ${MALLOC_LIB})

endfunction(faasm_func)

faasm_func(poly_covariance datamining/covariance.c)
