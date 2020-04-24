find_package(PythonLibs)

include_directories(${FAASM_INCLUDE_DIR}/runner)

set(LIB_FILES
        function.cpp
        Profiler.cpp
        GenericFunctionProfiler.cpp
        ${FAASM_INCLUDE_DIR}/runner/function.h
        ${FAASM_INCLUDE_DIR}/runner/Profiler.h
        )

faasm_private_lib(bench "${LIB_FILES}")
target_link_libraries(bench worker_lib wasm wavmmodule util module_cache system)

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner bench)

add_executable(simple_runner simple_runner.cpp)
target_link_libraries(simple_runner bench)

add_executable(func_sym func_sym.cpp)
target_link_libraries(func_sym bench)

