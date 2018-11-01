set(FAASM_FUNCTION open_blocked.cpp)

add_executable(native_err native_error.cpp ${FAASM_FUNCTION})

target_link_libraries(native_err)
