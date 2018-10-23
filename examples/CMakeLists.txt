include_directories(${FAASM_INCLUDE_DIR}/faasm)

set(FAASM_FUNCTION sgd_epoch.cpp)
add_executable(native_func native_runner.cpp ${FAASM_FUNCTION})

target_link_libraries(native_func curl)
