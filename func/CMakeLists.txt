include_directories(${FAASM_INCLUDE_DIR}/faasm ${FAASM_INCLUDE_DIR}/emscripten)

set(FAASM_FUNCTION function_curl.c)

add_executable(native_func native_runner.c ${FAASM_FUNCTION})

target_link_libraries(native_func curl)
