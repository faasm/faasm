set(FAASM_USER python)

# Normally we can rely on CMake to find the python libs, but when doing a
# wasm build we need to point to the wasm-built versions on the local machine
if (${EMSCRIPTEN})
    set(PYTHON_LIBRARIES "$ENV{HOME}/faasm/pyodide/cpython/installs/python-3.7.0/lib/libpython3.7.a")
    set(PYTHON_INCLUDE_DIRS "$ENV{HOME}/faasm/pyodide/cpython/installs/python-3.7.0/include/python3.7")
else ()
    find_package(PythonLibs)
endif ()

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})
faasm_func(py_hello py_hello.c)
target_link_libraries(py_hello ${PYTHON_LIBRARIES})

faasm_func(c_hello c_hello.c)


