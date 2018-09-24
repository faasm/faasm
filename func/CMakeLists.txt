include_directories(${FAASM_INCLUDE_DIR}/faasm)

set(FAASM_FUNCTION function_net.c)

add_executable(native_func native_runner.c ${FAASM_FUNCTION})

target_link_libraries(native_func curl)
