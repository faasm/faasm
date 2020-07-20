include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/storage/*.h")

set(LIB_FILES
        FileDescriptor.cpp
        FileLoader.cpp
        FileSystem.cpp
        FileserverFileLoader.cpp
        IBMFileLoader.cpp
        IBMStorageWrapper.cpp
        LocalFileLoader.cpp
        SharedFiles.cpp
        SparseMatrixFileSerialiser.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage faasm emulator util libWAVM)

include_wamr_non_sgx()
target_link_libraries(storage wamrmodule)
