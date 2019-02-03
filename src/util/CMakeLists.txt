include_directories(${FAASM_INCLUDE_DIR}/util)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/util/*.h")

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
        ${HEADERS})

target_link_libraries(
        util
        proto
        ssl
        crypto
        boost_system
        boost_filesystem
)
