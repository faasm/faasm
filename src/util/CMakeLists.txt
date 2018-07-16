include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util SHARED STATIC util.cpp ${FAASM_INCLUDE_DIR}/util/util.h)

install(TARGETS util DESTINATION lib)
install(FILES ${FAASM_INCLUDE_DIR}/util/util.h DESTINATION include/util)
