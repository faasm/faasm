include_directories(${FAASM_INCLUDE_DIR}/data)

add_library(data STATIC
        matrix.cpp
        local_sgd.cpp
        ${FAASM_INCLUDE_DIR}/data/data.h
        )

target_link_libraries(
        data
        faasm
        emulator
        prof
        util
)

add_executable(reuters_parse reuters_parse.cpp)
target_link_libraries(reuters_parse data)

add_executable(reuters_svm reuters_svm.cpp)
target_link_libraries(reuters_svm data)

add_executable(reuters_exp reuters_exp.cpp)
target_link_libraries(reuters_exp data)
