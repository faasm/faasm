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
        LocalFileLoader.cpp
        SharedFiles.cpp
        SparseMatrixFileSerialiser.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage faasm emulator util wavmmodule)

# WAMR module depends on SGX support
if(FAASM_SGX_SUPPORT)
    target_link_libraries(storage sgx_wamr)
else()
    target_link_libraries(storage wamrmodule)
endif()