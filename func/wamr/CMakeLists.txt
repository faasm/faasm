#
# Created by Joshua Heinemann on 13.06.20.
# TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
#
set(FAASM_USER demo)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-undefined -Wl,--no-entry -Wl,--export=main -nostdlib")

function(demo_wamr_func func_name source_file)
    add_executable(${func_name} ${source_file})
    target_link_libraries(${func_name})
endfunction(demo_wamr_func)

demo_wamr_func(hello_world hello_world.cpp)
demo_wamr_func(hello_world_sgx_wamr hello_world_sgx_wamr.cpp)
demo_wamr_func(puts puts.cpp)