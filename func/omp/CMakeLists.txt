set(FAASM_USER omp)

set(CMAKE_C_FLAGS "-fopenmp ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}")

add_executable(hellomp hellomp.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    target_link_libraries(hellomp faasmp faasm)
else ()
    target_link_libraries(hellomp pthread)
endif ()
