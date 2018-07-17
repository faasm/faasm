include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util SHARED STATIC util.cpp ${FAASM_INCLUDE_DIR}/util/util.h)
