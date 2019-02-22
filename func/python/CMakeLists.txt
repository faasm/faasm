set(FAASM_USER python)

# Normally we can rely on CMake to find the python libs, but when doing a
# wasm build we need to be specific
if(FAASM_BUILD_TYPE STREQUAL "wasm")
    set(PYTHON_INCLUDE_DIRS "/toolchain/sysroot/include/python3.6")
    set(PYTHON_LIBRARIES "/toolchain/sysroot/lib/libpython3.6.a")
else()
    find_package(PythonLibs)
endif()

# Python headers
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_func(py_hello py_hello.cpp)
target_link_libraries(py_hello ${PYTHON_LIBRARIES})

