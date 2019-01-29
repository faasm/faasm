include_directories(${FAASM_INCLUDE_DIR}/redis)

add_library(
        redis STATIC
        redis.cpp
        ${FAASM_INCLUDE_DIR}/redis/redis.h
)

target_link_libraries(
        redis
        util
        prof
        pthread
        hiredis
)
