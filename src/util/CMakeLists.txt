include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC util.cpp ${FAASM_INCLUDE_DIR}/util/util.h)
