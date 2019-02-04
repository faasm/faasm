include_directories(${FAASM_INCLUDE_DIR}/scheduler)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/scheduler/*.h")

add_library(scheduler STATIC
        AWSMessageBus.cpp
        GlobalMessageBus.cpp
        RedisMessageBus.cpp
        Scheduler.cpp
        SharingMessageBus.cpp
        ${HEADERS}
        )

target_link_libraries(
        scheduler
        redis
)
