include_directories(${FAASM_INCLUDE_DIR}/scheduler)

add_library(scheduler STATIC
        message_queue.cpp
        scheduler.cpp
        ${FAASM_INCLUDE_DIR}/scheduler/scheduler.h
        )

target_link_libraries(
        scheduler
        redis
)
