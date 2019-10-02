include_directories(
        ${FAASM_INCLUDE_DIR}/ibm
)

set(LIB_FILES
        ${FAASM_INCLUDE_DIR}/ibm/IBMEndpoint.h
        IBMEndpoint.cpp
        )

faasm_private_lib(worker_ibm_lib "${LIB_FILES}")

add_executable(worker_ibm
        worker_ibm.cpp
        )

target_link_libraries(worker_ibm
        endpoint
        worker_lib
        worker_ibm_lib
        )
