include_directories(${FAASM_INCLUDE_DIR}/emulator)

add_library(
        emulator STATIC
        emulator.cpp
)

target_link_libraries(emulator infra state)
