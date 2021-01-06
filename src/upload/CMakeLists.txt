include_directories(${FAASM_INCLUDE_DIR}/upload)

set(LIB_FILES
        UploadServer.cpp
        ${FAASM_INCLUDE_DIR}/upload/UploadServer.h
        )

faasm_private_lib(upload_lib "${LIB_FILES}")

target_link_libraries(upload_lib
        wasm
        wavmmodule
        faabric
        cpprest
        )

add_executable(upload upload_server.cpp)
target_link_libraries(upload upload_lib)
