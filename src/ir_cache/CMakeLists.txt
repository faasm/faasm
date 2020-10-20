include_directories(
        ${FAASM_INCLUDE_DIR}/ir_cache
)

set(LIB_FILES
    ${FAASM_INCLUDE_DIR}/ir_cache/IRModuleCache.h
    IRModuleCache.cpp
)

faasm_private_lib(ir_cache "${LIB_FILES}")
target_link_libraries(ir_cache libWAVM faabric)
