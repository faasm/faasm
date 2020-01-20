set(FAASM_USER polybench)

set(CMAKE_C_FLAGS "-fopenmp ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")

add_executable(hellomp hellomp.cpp)

target_link_libraries(hellomp pthread)
