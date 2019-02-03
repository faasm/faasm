include_directories(${FAASM_INCLUDE_DIR}/emulator)

add_library(
        emulator STATIC
        emulator.cpp
        ${FAASM_INCLUDE_DIR}/emulator/emulator.h
)

target_link_libraries(emulator scheduler state)
