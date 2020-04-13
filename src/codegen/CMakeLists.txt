add_executable(codegen_shared_obj codegen_shared_obj.cpp)
target_link_libraries(codegen_shared_obj wasm wavmmodule)

add_executable(codegen_func codegen_func.cpp)
target_link_libraries(codegen_func wasm wavmmodule)

