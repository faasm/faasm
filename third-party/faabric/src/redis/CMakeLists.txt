include_directories(${FAABRIC_INCLUDE_DIR}/faabric/redis)

set(LIB_FILES
        Redis.cpp
        ${FAABRIC_INCLUDE_DIR}/faabric/redis/Redis.h
    )

faasm_private_lib(redis ${LIB_FILES})

target_link_libraries(redis util pthread hiredis)
