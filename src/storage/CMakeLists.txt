include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}/Lib
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/storage/*.h")

set(LIB_FILES
        FileDescriptor.cpp
        FileLoader.cpp
        FileSystem.cpp
        FileserverFileLoader.cpp
        LocalFileLoader.cpp
        SharedFiles.cpp
        SparseMatrixFileSerialiser.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage faasm emulator wavmmodule wamrmodule)
