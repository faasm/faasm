include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC
        bytes.cpp
        config.cpp
        clock.cpp
        environment.cpp
        files.cpp
        hash.cpp
        logging.cpp
        network.cpp
        random.cpp
        strings.cpp
        tokens.cpp
        ${FAASM_INCLUDE_DIR}/util/util.h)

target_link_libraries(
        util
        ssl
        crypto
        boost_system
        boost_filesystem
)
