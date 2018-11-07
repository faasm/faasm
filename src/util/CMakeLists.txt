include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC
        bytes.cpp
        environment.cpp
        files.cpp
        logging.cpp
        network.cpp
        random.cpp
        tokens.cpp
        ${FAASM_INCLUDE_DIR}/util/util.h)
