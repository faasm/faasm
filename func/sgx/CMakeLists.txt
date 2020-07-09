set(FAASM_USER sgx)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-undefined -Wl,--no-entry -nostdlib")

function(demo_sgx_func func_name source_file)
    add_executable(${func_name} ${source_file})
    target_link_libraries(${func_name})
    target_compile_options(${func_name} PRIVATE "-fvisibility=hidden" "-I/usr/local/code/faasm/include") #TODO: Improve include path, just for testing purpose
    target_link_options(${func_name} PRIVATE "-Wl,-export-dynamic")
endfunction(demo_sgx_func)

demo_sgx_func(hello_world hello_world.cpp)
demo_sgx_func(hello_world_sgx_wamr hello_world_sgx_wamr.cpp)
demo_sgx_func(puts puts.cpp)
demo_sgx_func(puts_sgx_wamr puts_sgx_wamr.cpp)
demo_sgx_func(chaining_sgx_wamr chaining_sgx_wamr.cpp)