set(FAASM_USER python)

# Normally we can rely on CMake to find the python libs, but when doing a
# wasm build we need to point to the wasm-built versions on the local machine
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # These functions need to support dynamic linking
    set(CMAKE_CXX_FLAGS "${DYNLINK_EXE_FLAGS}")
    set(CMAKE_C_FLAGS "${DYNLINK_EXE_FLAGS}")

    set(PYODIDE_CPYTHON "/usr/local/code/pyodide/cpython/installs/python-3.7.0")

    set(PYTHON_LIBRARIES "${PYODIDE_CPYTHON}/lib/libpython3.7.a")
    set(PYTHON_INCLUDE_DIRS "${PYODIDE_CPYTHON}/include/python3.7")
else ()
    find_package(PythonLibs)
endif ()

message(STATUS "PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_func(py_hello py_hello.cpp)
target_link_libraries(py_hello ${PYTHON_LIBRARIES})
faasm_func(py_numpy py_numpy.cpp)
target_link_libraries(py_hello ${PYTHON_LIBRARIES})
