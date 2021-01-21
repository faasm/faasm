set(FAASM_USER python)

# Note - these are put in place as part of the container build
if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    set(PYTHON_LIBRARIES ${CMAKE_SYSROOT}/lib/wasm32-wasi/libpython3.8.a)
    set(PYTHON_INCLUDE_DIRS ${CMAKE_SYSROOT}/include/python3.8)
else ()
    find_package(PythonLibs)
endif ()

# Python functions
include_directories(${PYTHON_INCLUDE_DIRS})

add_executable(py_func pyinit.c pyinit.h py_func.cpp)

# Need to export all symbols to support dynamic linking C-extensions
set_target_properties(py_func PROPERTIES
    LINK_FLAGS "-Xlinker --export-all -Xlinker --no-gc-sections"                
    )

target_link_libraries(py_func faasm emscripten c-printscan-long-double)

if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    target_link_libraries(py_func ${PYTHON_LIBRARIES})

    # BLAS
    # target_link_libraries(py_func lapack blas cblas f2c)
else()
    target_link_libraries(py_func ${PYTHON_LIBRARIES} util)

    target_include_directories(py_func PRIVATE ${FAASM_NATIVE_INSTALL}/include)
endif()

# Custom target to support compiling pyhton function through compile.user task
add_custom_target(python_all_funcs DEPENDS py_func)
