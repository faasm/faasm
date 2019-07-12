include_directories(
        ${FAASM_INCLUDE_DIR}/memory
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/memory/*.h")

set(LIB_FILES
        MemorySnapshot.cpp
        MemorySnapshotRegister.cpp
        ${HEADERS}
        )

faasm_private_lib(memory "${LIB_FILES}")

target_link_libraries(memory redis)