include_directories(${FAASM_INCLUDE_DIR}/http)

set(LIB_FILES
        HttpEndpoint.cpp
        ${FAASM_INCLUDE_DIR}/http/HttpEndpoint.h
        )

faasm_private_lib(http "${LIB_FILES}")

target_link_libraries(http pistache)
