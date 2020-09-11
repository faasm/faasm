include_directories(${FAASM_INCLUDE_DIR}/wasm/openmp)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/wasm/openmp/*.h")

set(LIB_FILES
        Level.cpp
        ThreadState.cpp
        ${HEADERS}
        )

faasm_private_lib(openmp "${LIB_FILES}")
add_dependencies(openmp faabric)