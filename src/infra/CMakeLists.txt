include_directories(${FAASM_INCLUDE_DIR}/infra)

add_library(
        infra STATIC
        redis.cpp
        func.cpp
        ${FAASM_INCLUDE_DIR}/infra/infra.h
)

target_link_libraries(infra util proto cpp_redis tacopie pthread hiredis)
