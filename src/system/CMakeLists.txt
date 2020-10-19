include_directories(
        ${FAASM_INCLUDE_DIR}/system
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/system/*.h")

set(LIB_FILES
        CGroup.cpp
        NetworkNamespace.cpp
        ${HEADERS}
        )

add_library(system "${LIB_FILES}")
add_dependencies(system faabric_ext)
target_link_libraries(system faabric)
