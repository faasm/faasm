include_directories(${FAASM_INCLUDE_DIR}/data)

set(LIB_FILES
        matrix.cpp
        local_sgd.cpp
        ${FAASM_INCLUDE_DIR}/data/data.h
)

faasm_private_lib(data "${LIB_FILES}")

target_link_libraries(
        data
        faasm
        emulator
        util
)

add_executable(reuters_parse reuters_parse.cpp)
target_link_libraries(reuters_parse data)
