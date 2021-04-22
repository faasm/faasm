set(RUNNER_LIBS
        faaslet_lib
        wavmmodule
        wamrmodule
        module_cache
        system
        )

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner ${RUNNER_LIBS})

add_executable(lammps_runner lammps_runner.cpp)
target_link_libraries(lammps_runner ${RUNNER_LIBS})

add_executable(simple_runner simple_runner.cpp)
target_link_libraries(simple_runner ${RUNNER_LIBS})

add_executable(func_sym func_sym.cpp)
target_link_libraries(func_sym ${RUNNER_LIBS})

add_executable(codegen_shared_obj codegen_shared_obj.cpp)
target_link_libraries(codegen_shared_obj ${RUNNER_LIBS})

add_executable(codegen_func codegen_func.cpp)
target_link_libraries(codegen_func ${RUNNER_LIBS})

# Main entrypoint for worker nodes
add_executable(pool_runner pool_runner.cpp)
target_link_libraries(pool_runner ${RUNNER_LIBS})
