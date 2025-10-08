
faasm_private_lib(runner_lib
    MicrobenchRunner.cpp
    runner_utils.cpp
)
target_link_libraries(runner_lib PUBLIC
    faasm::common_deps
    faasm::faaslet_lib
    faasm::wavmmodule
    faasm::wamrmodule
    faasm::upload_lib
    faasm::system
)
target_include_directories(runner_lib PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_executable(func_runner func_runner.cpp)
target_link_libraries(func_runner PRIVATE faasm::runner_lib)
target_include_directories(func_runner PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_executable(func_sym func_sym.cpp)
target_link_libraries(func_sym PRIVATE faasm::runner_lib)
target_include_directories(func_sym PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_executable(microbench_runner microbench_runner.cpp)
target_link_libraries(microbench_runner PRIVATE faasm::runner_lib)
target_include_directories(microbench_runner PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_executable(local_pool_runner local_pool_runner.cpp)
target_link_libraries(local_pool_runner PRIVATE faasm::runner_lib)
target_include_directories(local_pool_runner PRIVATE ${FAASM_INCLUDE_DIR}/runner)

# Main entrypoint for worker nodes
add_executable(pool_runner pool_runner.cpp)
target_link_libraries(pool_runner PRIVATE faasm::runner_lib)
target_include_directories(pool_runner PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_library(codegen_common INTERFACE)
add_library(faasm::codegen_common ALIAS codegen_common)
target_link_libraries(codegen_common INTERFACE
    faasm::wasm_codegen
    faasm::runner_lib
)

add_executable(codegen_shared_obj codegen_shared_obj.cpp)
target_link_libraries(codegen_shared_obj PRIVATE faasm::codegen_common)
target_include_directories(codegen_shared_obj PRIVATE ${FAASM_INCLUDE_DIR}/runner)

add_executable(codegen_func codegen_func.cpp)
target_link_libraries(codegen_func PRIVATE faasm::codegen_common)
target_include_directories(codegen_func PRIVATE ${FAASM_INCLUDE_DIR}/runner)
