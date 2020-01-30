include_directories(${FAASM_INCLUDE_DIR}/mpi)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/mpi/*.h")

set(LIB_FILES
        MpiContext.cpp
        MpiWorldRegistry.cpp
        MpiWorld.cpp
        ${HEADERS}
        )

faasm_private_lib(mpi "${LIB_FILES}")
target_link_libraries(mpi scheduler state)
