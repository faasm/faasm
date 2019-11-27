include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}
)

set(HEADERS
        ${FAASM_INCLUDE_DIR}/storage/FileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/FileserverFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/IBMFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/IBMStorageWrapper.h
        ${FAASM_INCLUDE_DIR}/storage/LocalFileLoader.h
        ${FAASM_INCLUDE_DIR}/storage/SharedFilesManager.h
        )

set(LIB_FILES
        FileLoader.cpp
        LocalFileLoader.cpp
        FileserverFileLoader.cpp
        IBMFileLoader.cpp
        IBMStorageWrapper.cpp
        SharedFilesManager.cpp
        instance.cpp
        ${HEADERS}
        )

if(FAASM_AWS_SUPPORT)
    set(HEADERS ${FAASM_INCLUDE_DIR}/storage/S3FileLoader.h ${HEADERS})
    set(LIB_FILES S3FileLoader.cpp ${LIB_FILES})
    set(EXTRA_LIBS aws)
endif()

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage util libWAVM ${EXTRA_LIBS})
