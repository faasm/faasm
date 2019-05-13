find_package(PythonLibs)

add_executable(func_runner runner.cpp)
target_link_libraries(func_runner wasm prof)

add_executable(func_sym map_symbols.cpp)
target_link_libraries(func_sym wasm)

include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(python_bench python_benchmarker.cpp)
target_link_libraries(python_bench wasm prof ${PYTHON_LIBRARIES})
