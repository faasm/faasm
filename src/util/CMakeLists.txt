include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC
        bytes.cpp
        config.cpp
        clock.cpp
        environment.cpp
        files.cpp
        func.cpp
        hash.cpp
        logging.cpp
        memory.cpp
        network.cpp
        queue.cpp
        random.cpp
        strings.cpp
        ${FAASM_INCLUDE_DIR}/util/util.h)

target_link_libraries(
        util
        proto
        ssl
        crypto
        boost_system
        boost_filesystem
)
