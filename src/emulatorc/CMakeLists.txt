include_directories(${FAASM_INCLUDE_DIR}/emulator)

add_library(
        emulatorc STATIC
        emulatorc.c
        ${FAASM_INCLUDE_DIR}/emulator/emulator.h
)

