include_directories(
    ${FAASM_INCLUDE_DIR}/runner
)

set(LIB_FILES
    "${FAASM_INCLUDE_DIR}/runner/MicrobenchRunner.h"
    MicrobenchRunner.cpp
)

faasm_private_lib(runner_lib "${LIB_FILES}")
target_link_libraries(runner_lib
    faaslet_lib
    upload_lib
    system
)

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner runner_lib)

add_executable(func_sym func_sym.cpp)
target_link_libraries(func_sym runner_lib)

add_executable(microbench_runner microbench_runner.cpp)
target_link_libraries(microbench_runner runner_lib)

# Main entrypoint for worker nodes
add_executable(pool_runner pool_runner.cpp)
target_link_libraries(pool_runner runner_lib)

set(CODEGEN_LIBS
    codegen
    system
)

add_executable(codegen_shared_obj codegen_shared_obj.cpp)
target_link_libraries(codegen_shared_obj ${CODEGEN_LIBS})

add_executable(codegen_func codegen_func.cpp)
target_link_libraries(codegen_func ${CODEGEN_LIBS})
