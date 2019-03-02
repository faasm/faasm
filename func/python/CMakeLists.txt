set(FAASM_USER python)

# Normally we can rely on CMake to find the python libs, but when doing a
# wasm build we need to point to the wasm-built versions on the local machine
if(FAASM_BUILD_TYPE STREQUAL "emscripten")
    set(PYTHON_LIBRARIES "/usr/local/code/cpython-emscripten/installs/python-3.5.2/lib/libpython3.5.a")
    set(PYTHON_INCLUDE_DIRS "/usr/local/code/cpython-emscripten/installs/python-3.5.2/include/python3.5")
else()
    find_package(PythonLibs)
endif()

# Python headers
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_func(py_hello py_hello.cpp)
target_link_libraries(py_hello ${PYTHON_LIBRARIES})

