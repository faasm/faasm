set(FAASM_USER python)

set(PYODIDE_CPYTHON "/usr/local/code/faasm/third-party/pyodide/cpython/installs/python-3.7.0")

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # Second terms here for zlib
    set(PYTHON_LIBRARIES ${PYODIDE_CPYTHON}/lib/libpython3.7m.a ${PYODIDE_CPYTHON}/lib/libz.a)
    set(PYTHON_INCLUDE_DIRS ${PYODIDE_CPYTHON}/include/python3.7m ${PYODIDE_CPYTHON}/include)
else ()
    find_package(PythonLibs)
endif ()

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_dynamic_func(py_func py_func.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # In wasm build we need to also link with clapack
    target_link_libraries(py_func
            pyinit
            ${PYTHON_LIBRARIES}
            lapack
            blas
            f2c
            )
elseif (FAASM_BUILD_TYPE STREQUAL "knative-native")
    target_link_libraries(py_func-knative pyinit ${PYTHON_LIBRARIES} util)
else()
    target_link_libraries(py_func pyinit ${PYTHON_LIBRARIES} util)
endif()
