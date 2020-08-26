include_directories(
        ${FAASM_INCLUDE_DIR}/system
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/system/*.h")

set(LIB_FILES
        CGroup.cpp
        NetworkNamespace.cpp
        SGX.cpp
        ${HEADERS}
        )

add_library(system "${LIB_FILES}")
target_link_libraries(system faabric)
