
faasm_private_lib(faaslet_lib
    Faaslet.cpp
)
target_include_directories(faaslet_lib PRIVATE ${FAASM_INCLUDE_DIR}/faaslet)
target_link_libraries(faaslet_lib PUBLIC
    faasm::system
    faasm::threads
    faasm::storage
)

# Include SGX lib if enabled
if(SGX_FOUND)
    target_link_libraries(faaslet_lib PUBLIC faasm::enclave)
endif()
