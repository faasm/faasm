set(FAASM_USER python)

# Normally we can rely on CMake to find the python libs, but when doing a
# wasm build we need to point to the wasm-built versions on the local machine
if (FAASM_BUILD_TYPE STREQUAL "pyodide")
    set(PYODIDE_CPYTHON "/usr/local/code/pyodide/cpython/installs/python-3.7.0")
    set(PYODIDE_PKG "/usr/local/code/pyodide/packages")

    set(PYTHON_LIBRARIES "${PYODIDE_CPYTHON}/lib/libpython3.7.a")
    set(PYTHON_INCLUDE_DIRS "${PYODIDE_CPYTHON}/include/python3.7")
    set(NUMPY_INSTALL "${PYODIDE_PKG}/numpy/build/numpy-1.15.1/install/lib/python3.7/site-packages/numpy")

    set(NUMPY_INCLUDE_DIR "${NUMPY_INSTALL}/core/include")
    file(GLOB_RECURSE NUMPY_LIBS "${NUMPY_INSTALL}/**.so")

elseif(${EMSCRIPTEN})
    set(PYTHON_LIBRARIES "/usr/local/code/faasm/cpython-emscripten/installs/python-3.5.2/lib/libpython3.5.a")
    set(PYTHON_INCLUDE_DIRS "/usr/local/code/faasm/cpython-emscripten/installs/python-3.5.2/include/python3.5")
    set(NUMPY_LIBS "")
else ()
    find_package(PythonLibs)
    set(NUMPY_LIBS "")
endif ()

message(STATUS "PYTHON_LIBRARIES=${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
message(STATUS "NUMPY_LIBS=${NUMPY_LIBS}")
message(STATUS "NUMPY_INCLUDE_DIR=${NUMPY_INCLUDE_DIR}")

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS} ${NUMPY_INCLUDE_DIR})
faasm_func(py_hello py_hello.cpp)
target_link_libraries(py_hello ${PYTHON_LIBRARIES} ${NUMPY_LIBS})

faasm_func(c_hello c_hello.c)


