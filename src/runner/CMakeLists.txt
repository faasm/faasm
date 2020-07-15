set(RUNNER_LIBS
        faaslet_lib
        wasm
        wavmmodule
        util
        module_cache
        system
        )

set(BUILD_UNTRUST_PART 1)
include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner ${RUNNER_LIBS})

add_executable(simple_runner simple_runner.cpp)
target_link_libraries(simple_runner ${RUNNER_LIBS})

add_executable(func_sym func_sym.cpp)
target_link_libraries(func_sym ${RUNNER_LIBS})
