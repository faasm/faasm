include_directories(${FAASM_INCLUDE_DIR}/python)

add_library(python STATIC
        PythonWrapper.cpp
        python.cpp
        ${FAASM_INCLUDE_DIR}/python/PythonWrapper.h
        )

target_link_libraries(
        python
        wasm
)

add_executable(python_run python.cpp)
target_link_libraries(python_run python)
