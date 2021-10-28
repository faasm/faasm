
faasm_private_lib(system
    CGroup.cpp
    NetworkNamespace.cpp
    memory.cpp
)
target_include_directories(system PRIVATE ${FAASM_INCLUDE_DIR}/system)
