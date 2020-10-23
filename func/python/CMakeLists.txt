set(FAASM_USER python)

# Note - these are put in place as part of the container build
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(PYTHON_LIBRARIES ${CMAKE_SYSROOT}/lib/wasm32-wasi/libpython3.8.a)
    set(PYTHON_INCLUDE_DIRS ${CMAKE_SYSROOT}/include/python3.8)
else ()
    find_package(PythonLibs)
endif ()

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_dynamic_func(py_func py_func.cpp)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    target_link_libraries(py_func pyinit ${PYTHON_LIBRARIES})

    # BLAS
    # target_link_libraries(py_func lapack blas cblas f2c)
else()
    target_link_libraries(py_func pyinit ${PYTHON_LIBRARIES} util)
endif()
