include_directories(${FAABRIC_INCLUDE_DIR}/faabric/util)

find_package(RapidJSON)

file(GLOB HEADERS "${FAABRIC_INCLUDE_DIR}/faabric/util/*.h")

set(LIB_FILES
        barrier.cpp
        bytes.cpp
        chaining.cpp
        config.cpp
        clock.cpp
        environment.cpp
        files.cpp
        func.cpp
        gids.cpp
        json.cpp
        logging.cpp
        memory.cpp
        network.cpp
        queue.cpp
        random.cpp
        state.cpp
        string_tools.cpp
        timing.cpp
        ${HEADERS}
        )

faasm_private_lib(util "${LIB_FILES}")

target_link_libraries(
        util
        proto
        ssl
        curl
        crypto
        boost_system
        boost_filesystem
)
