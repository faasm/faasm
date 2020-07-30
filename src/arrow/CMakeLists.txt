include_directories(${FAASM_INCLUDE_DIR}/arrow)

find_package(Arrow REQUIRED)

message(STATUS "Found Arrow ${ARROW_VERSION}")

set(PUBLIC_HEADERS
        ${FAASM_INCLUDE_DIR}/arrow/arrow.h
        )

set(LIB_FILES
        arrow.cpp
        )

faasm_public_lib(arrow "${LIB_FILES}")
target_link_libraries(arrow PRIVATE arrow_shared)

set_target_properties(arrow PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

