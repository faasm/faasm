include_directories(
        ${FAASM_INCLUDE_DIR}/module_cache
)

set(LIB_FILES
    ${FAASM_INCLUDE_DIR}/module_cache/WasmModuleCache.h
    WasmModuleCache.cpp
)

faasm_private_lib(module_cache "${LIB_FILES}")
target_link_libraries(module_cache wasm wavmmodule)
