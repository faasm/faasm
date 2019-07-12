include_directories(${FAASM_INCLUDE_DIR}/scheduler)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/scheduler/*.h")

set(HEADERS
        ${FAASM_INCLUDE_DIR}/scheduler/GlobalMessageBus.h
        ${FAASM_INCLUDE_DIR}/scheduler/InMemoryMessageQueue.h
        ${FAASM_INCLUDE_DIR}/scheduler/RedisMessageBus.h
        ${FAASM_INCLUDE_DIR}/scheduler/Scheduler.h
        ${FAASM_INCLUDE_DIR}/scheduler/SharingMessageBus.h
)

set(LIB_FILES
        GlobalMessageBus.cpp
        RedisMessageBus.cpp
        Scheduler.cpp
        SharingMessageBus.cpp
        ${HEADERS}
)

if(FAASM_AWS_SUPPORT)
    set(HEADERS ${FAASM_INCLUDE_DIR}/scheduler/AWSMessageBus.h ${HEADERS})
    set(LIB_FILES AWSMessageBus.cpp ${LIB_FILES})
    set(EXTRA_LIBS aws)
endif()

faasm_private_lib(scheduler "${LIB_FILES}")
target_link_libraries(scheduler redis ${EXTRA_LIBS})
