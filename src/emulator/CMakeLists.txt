include_directories(${FAASM_INCLUDE_DIR}/emulator)

add_library(
        emulator STATIC
        emulator.cpp
        ${FAASM_LIB_DIR}/faasm/faasm.h
)

target_link_libraries(emulator infra)
