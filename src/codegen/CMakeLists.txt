include_directories(
    ${FAASM_INCLUDE_DIR}/codegen
)

set(HEADERS
    "${FAASM_INCLUDE_DIR}/codegen/MachineCodeGenerator.h"
    )

set(LIB_FILES
    MachineCodeGenerator.cpp
    ${HEADERS}
    )

faasm_private_lib(codegen "${LIB_FILES}")

target_link_libraries(codegen
    storage
)
