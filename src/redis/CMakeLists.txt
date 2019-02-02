include_directories(${FAASM_INCLUDE_DIR}/redis)

add_library(
        redis STATIC
        Redis.cpp
        ${FAASM_INCLUDE_DIR}/redis/Redis.h
)

target_link_libraries(
        redis
        util
        prof
        pthread
        hiredis
)
