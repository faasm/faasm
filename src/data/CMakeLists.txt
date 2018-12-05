include_directories(${FAASM_INCLUDE_DIR}/data)

add_library(data STATIC
        matrix.cpp
        ${FAASM_INCLUDE_DIR}/data/data.h
        )

target_link_libraries(
        data
        faasm
        emulator
        util
)

add_executable(reuters reuters.cpp)

target_link_libraries(reuters data)
