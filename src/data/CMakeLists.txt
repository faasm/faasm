include_directories(${FAASM_INCLUDE_DIR}/data)

add_executable(
        reuters
        matrix.cpp
        parse.cpp
)

target_link_libraries(
        reuters
        faasm
        emulator
        util
)
