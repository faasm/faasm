set(FAASM_USER errors)

function(error_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})

    set(ALL_ERROR_FUNCS ${ALL_ERROR_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(error_func)

error_func(bad_malloc bad_malloc.cpp)
error_func(munmap munmap.cpp)
error_func(open_blocked open_blocked.cpp)
error_func(ret_one ret_one.cpp)
error_func(stdin_poll stdin_poll.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    error_func(stackoverflow stackoverflow.cpp)
endif()

add_custom_target(errors_all_funcs DEPENDS ${ALL_ERROR_FUNCS})
