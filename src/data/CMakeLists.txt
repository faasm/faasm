include_directories(${FAASM_INCLUDE_DIR}/reuters)

add_executable(reuters parse.cpp)

target_link_libraries(reuters
        faasm
        util
        )
