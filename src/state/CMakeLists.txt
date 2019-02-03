include_directories(
        ${FAASM_INCLUDE_DIR}/state
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/state/*.h")

add_library(state STATIC
        State.cpp
        UserState.cpp
        StateKeyValue.cpp
        ${HEADERS}
        )

target_link_libraries(state proto redis Logging)
