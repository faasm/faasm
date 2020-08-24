include_directories(${FAABRIC_INCLUDE_DIR}/scheduler)

file(GLOB HEADERS "${FAABRIC_INCLUDE_DIR}/scheduler/*.h")

set(LIB_FILES
        ExecGraph.cpp
        FunctionCallClient.cpp
        FunctionCallServer.cpp
        Scheduler.cpp
        MpiContext.cpp
        MpiWorldRegistry.cpp
        MpiWorld.cpp
        ${HEADERS}
)

faasm_private_lib(scheduler "${LIB_FILES}")
target_link_libraries(scheduler faasmpi redis)
