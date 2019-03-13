include_directories(
        ${FAASM_INCLUDE_DIR}/memory
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/memory/*.h")

add_library(memory STATIC
        MemorySnapshot.cpp
        ${HEADERS}
        )

target_link_libraries(memory redis)
