find_package(PythonLibs)

include_directories(${FAASM_INCLUDE_DIR}/runner)

set(LIB_FILES
        function.cpp
        Profiler.cpp
        PythonProfiler.cpp
        PolybenchProfiler.cpp
        GenericFunctionProfiler.cpp
        ${FAASM_INCLUDE_DIR}/runner/function.h
        ${FAASM_INCLUDE_DIR}/runner/Profiler.h
        )

faasm_private_lib(bench "${LIB_FILES}")
target_link_libraries(bench wasm util zygote)

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner bench worker_lib)

add_executable(simple_runner simple_runner.cpp)
target_link_libraries(simple_runner bench worker_lib)

add_executable(func_sym map_symbols.cpp)
target_link_libraries(func_sym bench)

include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(python_bench python_bench.cpp)
target_link_libraries(python_bench bench ${PYTHON_LIBRARIES})

add_executable(poly_bench poly_bench.cpp)
target_link_libraries(poly_bench bench)

add_executable(bench_mem bench_mem.cpp)
target_link_libraries(bench_mem bench wasm zygote system)

add_executable(bench_func bench_func.cpp)
target_link_libraries(bench_func bench wasm zygote system)

add_executable(bench_time bench_time.cpp)
target_link_libraries(bench_time bench wasm zygote system)

add_executable(bench_tpt bench_tpt.cpp)
target_link_libraries(bench_tpt bench wasm zygote system)

add_executable(thread_bench_mem thread_bench_mem.cpp)
target_link_libraries(thread_bench_mem util)

add_executable(thread_bench_time thread_bench_time.cpp)
target_link_libraries(thread_bench_time util)
