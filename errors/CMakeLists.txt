include_directories(${FAASM_INCLUDE_DIR}/faasm)

set(FAASM_FUNCTION multi_poll.cpp)

add_executable(native_err native_error.cpp ${FAASM_FUNCTION})

target_link_libraries(native_err)
