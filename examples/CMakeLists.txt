set(FAASM_FUNCTION state.cpp)

add_executable(native_func
        native_runner.cpp
        ${FAASM_FUNCTION}
        ${FAASM_LIB_DIR}/faasm/faasm.h
        )

target_link_libraries(native_func faasm emulator util curl)
