set(FAASM_USER demo)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-undefined -Wl,--no-entry -Wl,--export=main -nostdlib")

function(demo_wamr_func func_name source_file)
    add_executable(${func_name} ${source_file})
    target_link_libraries(${func_name})
endfunction(demo_wamr_func)

demo_wamr_func(hello_world hello_world.cpp)