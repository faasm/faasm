include_directories(
        ${FAASM_INCLUDE_DIR}/storage
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/storage/*.h")

set(LIB_FILES
        FileLoader.cpp
        LocalFileLoader.cpp
        FileserverFileLoader.cpp
        IBMFileLoader.cpp
        S3FileLoader.cpp
        SharedFilesManager.cpp
        instance.cpp
        ${HEADERS}
        )

faasm_private_lib(storage "${LIB_FILES}")
target_link_libraries(storage util aws libWAVM)
