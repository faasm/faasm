include_directories(
        ${FAASM_INCLUDE_DIR}/knative_native
)

set(
        LIB_FILES
        ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeHandler.h
        ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeEndpoint.h
        KnativeNativeHandler.cpp
        KnativeNativeEndpoint.cpp
)

# Link with cpython libraries
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIRS})

faasm_private_lib(knative_native "${LIB_FILES}")

target_link_libraries(knative_native endpoint emulator scheduler ${PYTHON_LIBRARIES})

# Add an executable to run it locally
add_definitions("-DKNATIVE_NATIVE=1")
add_executable(knative_native_runner knative_native_runner.cpp)
target_link_libraries(knative_native_runner faasm knative_native)
