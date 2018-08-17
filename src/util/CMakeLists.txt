include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC
        bytes.cpp
        environment.cpp
        files.cpp
        random.cpp
        ${FAASM_INCLUDE_DIR}/util/util.h)
