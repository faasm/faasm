# Native implementation of functions that would be provided by faasm runtime

add_library(native STATIC
        state.cpp
        ${FAASM_INCLUDE_DIR}/faasm/faasm.h
        )

target_link_libraries(native infra)
