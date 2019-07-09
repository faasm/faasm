find_package(PythonLibs)

include_directories(${FAASM_INCLUDE_DIR}/runner)

add_library(bench STATIC
        timing.cpp
        Profiler.cpp
        PythonProfiler.cpp
        PolybenchProfiler.cpp
        ${FAASM_INCLUDE_DIR}/runner/Profiler.h
        ${FAASM_INCLUDE_DIR}/runner/timing.h
    )

target_link_libraries(bench wasm util)

add_executable(func_runner runner.cpp)
target_link_libraries(func_runner bench worker_lib)

add_executable(func_sym map_symbols.cpp)
target_link_libraries(func_sym bench)

include_directories(${PYTHON_INCLUDE_DIRS})
add_executable(python_bench python_bench.cpp)
target_link_libraries(python_bench bench ${PYTHON_LIBRARIES})

add_executable(poly_bench poly_bench.cpp)
target_link_libraries(poly_bench bench)