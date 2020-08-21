include_directories(${FAASM_INCLUDE_DIR}/redis)

set(LIB_FILES
        Redis.cpp
        ${FAASM_INCLUDE_DIR}/redis/Redis.h
    )

faasm_private_lib(redis ${LIB_FILES})

target_link_libraries(redis util pthread hiredis)
