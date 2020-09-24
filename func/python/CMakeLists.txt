set(FAASM_USER python)

set(FAASM_CPYTHON "/usr/local/code/faasm/third-party/faasm-cpython")
set(CPYTHON_INSTALL "${FAASM_CPYTHON}/third-party/cpython/install/wasm/")

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(PYTHON_LIBRARIES ${CPYTHON_INSTALL}/lib/libpython3.8.a)
    set(PYTHON_INCLUDE_DIRS ${CPYTHON_INSTALL}/include/python3.8)
else ()
    find_package(PythonLibs)
endif ()

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_dynamic_func(py_func py_func.cpp)
# faasm_func(py_func py_func.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    target_link_libraries(py_func pyinit ${PYTHON_LIBRARIES})

    # Printing long double support
    target_link_libraries(py_func c-printscan-long-double)

    # BLAS
    # target_link_libraries(py_func lapack blas cblas f2c)
else()
    target_link_libraries(py_func pyinit ${PYTHON_LIBRARIES} util)
endif()
