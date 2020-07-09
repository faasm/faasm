#
# Created by Joshua Heinemann on 13.06.20.
# TU-Braunschweig (heineman@ibr.cs.tu-bs.de)
#
set(FAASM_USER wamr)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-undefined -Wl,--no-entry -nostdlib")

function(demo_wamr_func func_name source_file)
    add_executable(${func_name} ${source_file})
    target_link_libraries(${func_name})
    target_compile_options(${func_name} PRIVATE "-fvisibility=hidden" "-I/usr/local/code/faasm/include") #TODO: Improve include path, just for testing purpose
    target_link_options(${func_name} PRIVATE "-Wl,-export-dynamic")
endfunction(demo_wamr_func)

demo_wamr_func(hello_world_sgx_wamr hello_world_sgx_wamr.cpp)
demo_wamr_func(chaining_internal_sgx_wamr chaining_internal_sgx_wamr.cpp)
demo_wamr_func(chaining_external_sgx_wamr chaining_external_sgx_wamr.cpp)
demo_wamr_func(chaining_external_func_sgx_wamr chaining_external_func_sgx_wamr.cpp)