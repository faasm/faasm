include_directories(${FAASM_INCLUDE_DIR}/endpoint)

set(LIB_FILES
        Endpoint.cpp
        ${FAASM_INCLUDE_DIR}/endpoint/Endpoint.h
        )

faasm_private_lib(endpoint "${LIB_FILES}")

target_link_libraries(endpoint pistache pthread faabric)
