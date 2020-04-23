include_directories(
        ${FAASM_INCLUDE_DIR}/state
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/state/*.h")

set(LIB_FILES
        State.cpp
        StateBackend.cpp
        RedisStateBackend.cpp
        InMemoryStateBackend.cpp
        UserState.cpp
        StateKeyValue.cpp
        ${HEADERS}
    )

faasm_private_lib(state "${LIB_FILES}")
target_link_libraries(state proto redis)
