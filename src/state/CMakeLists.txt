include_directories(
        ${FAASM_INCLUDE_DIR}/state
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(state STATIC
        state.cpp
        ${FAASM_INCLUDE_DIR}/state/state.h
        )

target_link_libraries(state infra Logging)
