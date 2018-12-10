include_directories(${FAASM_INCLUDE_DIR}/infra)

add_library(
        infra STATIC
        func.cpp
        redis.cpp
        scheduler.cpp
        ${FAASM_INCLUDE_DIR}/infra/infra.h
)

target_link_libraries(
        infra
        prof
        proto
        pthread
        hiredis
)
