set(FAASM_USER python)

set(PYODIDE_CPYTHON "/usr/local/code/faasm/pyodide/cpython/installs/python-3.7.0")

if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    # Second terms here for zlib
    set(PYTHON_LIBRARIES ${PYODIDE_CPYTHON}/lib/libpython3.7d.a ${PYODIDE_CPYTHON}/lib/libz.a)
    set(PYTHON_INCLUDE_DIRS ${PYODIDE_CPYTHON}/include/python3.7d ${PYODIDE_CPYTHON}/include)
else ()
    find_package(PythonLibs)
endif ()

message(STATUS "PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_dynamic_func(py_hello py_hello.cpp)
target_link_libraries(py_hello pyfaasm ${PYTHON_LIBRARIES})

faasm_dynamic_func(py_numpy py_numpy.cpp)
target_link_libraries(py_numpy pyfaasm ${PYTHON_LIBRARIES})
