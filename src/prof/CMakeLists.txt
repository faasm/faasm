include_directories(${FAASM_INCLUDE_DIR}/prof)

add_library(
        prof STATIC
        timing.cpp
        ${FAASM_INCLUDE_DIR}/prof/prof.h
)

target_link_libraries(prof util)
