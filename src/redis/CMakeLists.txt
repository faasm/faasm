include_directories(${FAASM_INCLUDE_DIR}/redis)

add_library(redis SHARED STATIC redis.cpp ${FAASM_INCLUDE_DIR}/redis/redis.h)

target_link_libraries(redis util cpp_redis pthread)

install(TARGETS redis DESTINATION lib)
install(FILES ${FAASM_INCLUDE_DIR}/redis/redis.h DESTINATION include/redis)
