set(EASY_PROFILER_INCLUDE /usr/local/code/lib/easy_profiler/easy_profiler_core/include)
set(EASY_PROFILER_LIB /usr/local/code/lib/easy_profiler/build/bin/libeasy_profiler.so)

find_package(PythonLibs)

add_executable(func_runner runner.cpp)
target_link_libraries(func_runner wasm prof)

include_directories(${PYTHON_INCLUDE_DIRS} ${EASY_PROFILER_INCLUDE})
add_executable(python_bench python_benchmarker.cpp)
target_link_libraries(python_bench wasm prof ${EASY_PROFILER_LIB} ${PYTHON_LIBRARIES})
