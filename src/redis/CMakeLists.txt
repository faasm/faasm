include_directories(${FAASM_INCLUDE_DIR}/redis)

add_library(redis SHARED redis.cpp redis.h)

target_link_libraries(redis cpp_redis pthread)

#install(TARGETS redis DESTINATION lib)
#install(FILES redis.h DESTINATION include/redis)