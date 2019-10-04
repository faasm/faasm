include_directories(
        ${FAASM_INCLUDE_DIR}/ibm
)

set(LIB_FILES
        ${FAASM_INCLUDE_DIR}/ibm/IBMEndpoint.h
        ${FAASM_INCLUDE_DIR}/ibm/IBMStorageWrapper.h
        IBMEndpoint.cpp
        IBMStorageWrapper.cpp
        )

faasm_private_lib(worker_ibm_lib "${LIB_FILES}")

target_link_libraries(worker_ibm_lib endpoint worker_lib)

add_executable(worker_ibm worker_ibm.cpp)

target_link_libraries(worker_ibm worker_ibm_lib)
