faasm_private_lib(wasm_codegen
    MachineCodeGenerator.cpp
)
target_include_directories(wasm_codegen PRIVATE ${FAASM_INCLUDE_DIR}/codegen)
target_link_libraries(wasm_codegen PUBLIC
    faasm::storage
    faasm::wamrmodule
    faasm::wavmmodule
)
