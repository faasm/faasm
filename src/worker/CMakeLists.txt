# include_directories(${FAASM_INCLUDE_DIR}/redis)

add_executable(worker worker.cpp)

target_link_libraries(worker redis util cpp_redis tacopie pthread)

set_target_properties(worker PROPERTIES FOLDER worker)