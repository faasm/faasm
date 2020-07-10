include_directories(
        ${FAASM_INCLUDE_DIR}/faaslet
)

set(LIB_HEADERS
        ${FAASM_INCLUDE_DIR}/faaslet/FaasmMain.h
        ${FAASM_INCLUDE_DIR}/faaslet/Faaslet.h
        ${FAASM_INCLUDE_DIR}/faaslet/FaasletPool.h
        )

set(LIB_FILES
        FaasmMain.cpp
        Faaslet.cpp
        FaasletPool.cpp
        ${LIB_HEADERS}
        )

faasm_private_lib(faaslet_lib "${LIB_FILES}")
target_link_libraries(faaslet_lib wasm wavmmodule module_cache system)

# Have to include WAMR stuff here
set(BUILD_UNTRUST_PART 1)
include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

target_link_libraries(faaslet_lib wamrmodule)

# Extra SGX WAMR support
if(FAASM_SGX_SUPPORT)
    target_link_libraries(faaslet_lib sgx_wamr)
endif()
