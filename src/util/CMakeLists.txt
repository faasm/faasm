include_directories(${FAASM_INCLUDE_DIR}/util)

add_library(util SHARED STATIC util.cpp messaging.cpp faasm.pb.cc ${FAASM_INCLUDE_DIR}/util/util.h faasm.pb.h)

install(TARGETS util DESTINATION lib)
install(FILES ${FAASM_INCLUDE_DIR}/util/util.h DESTINATION include/util)
