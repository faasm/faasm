faasm_private_lib(codegen
    MachineCodeGenerator.cpp
)
target_include_directories(codegen PRIVATE ${FAASM_INCLUDE_DIR}/codegen)
target_link_libraries(codegen PUBLIC
    faasm::storage
    faasm::wamrmodule
    faasm::wavmmodule
)
