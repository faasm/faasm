
add_executable(func_runner runner.cpp)
target_link_libraries(func_runner wasm)

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(native_cpython_runner native_cpython_runner.cpp)
target_link_libraries(native_cpython_runner pyfaasm ${PYTHON_LIBRARIES})
