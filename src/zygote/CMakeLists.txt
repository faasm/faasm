include_directories(
        ${FAASM_INCLUDE_DIR}/zygote
)

set(LIB_FILES
    ${FAASM_INCLUDE_DIR}/zygote/ZygoteRegistry.h
    ZygoteRegistry.cpp
)

faasm_private_lib(zygote "${LIB_FILES}")
target_link_libraries(zygote wasm)
