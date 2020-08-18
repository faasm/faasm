include_directories(${FAASM_INCLUDE_DIR}/scheduler)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/scheduler/*.h")

set(LIB_FILES
        ExecGraph.cpp
        FunctionCallClient.cpp
        FunctionCallServer.cpp
        Scheduler.cpp
        SchedulerHttpMixin.cpp
        ${HEADERS}
)

faasm_private_lib(scheduler "${LIB_FILES}")
target_link_libraries(scheduler redis)
