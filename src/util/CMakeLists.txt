include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util STATIC environment.cpp random.cpp bytes.cpp ${FAASM_INCLUDE_DIR}/util/util.h)
