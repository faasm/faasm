include_directories(${FAASM_INCLUDE_DIR}/util)

find_package(RapidJSON)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/util/*.h")

set(LIB_FILES
        base64.cpp
        bytes.cpp
        config.cpp
        clock.cpp
        environment.cpp
        files.cpp
        func.cpp
        hash.cpp
        json.cpp
        logging.cpp
        memory.cpp
        network.cpp
        queue.cpp
        random.cpp
        state.cpp
        strings.cpp
        ${HEADERS}
        )

faasm_private_lib(util "${LIB_FILES}")

target_link_libraries(
        util
        proto
        ssl
        crypto
        boost_system
        boost_filesystem
)
