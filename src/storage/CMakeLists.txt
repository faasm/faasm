include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}
)

set(HEADERS
        ${FAASM_INCLUDE_DIR}/storage/FileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/FileDescriptor.h
        ${FAASM_INCLUDE_DIR}/storage/FileSystem.h
        ${FAASM_INCLUDE_DIR}/storage/FileserverFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/IBMFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/IBMStorageWrapper.h
        ${FAASM_INCLUDE_DIR}/storage/LocalFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/SharedFiles.h
        )

set(LIB_FILES
        FileDescriptor.cpp
        FileLoader.cpp
        FileSystem.cpp
        FileserverFileLoader.cpp
        IBMFileLoader.cpp
        IBMStorageWrapper.cpp
        LocalFileLoader.cpp
        SharedFiles.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage util libWAVM ${EXTRA_LIBS})
