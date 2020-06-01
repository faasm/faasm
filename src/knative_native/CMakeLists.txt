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

# NOTE - if having issues with linking Faasm functions, build with FAASM_STATIC_LIBS=OFF
faasm_private_lib(knative_native "${LIB_FILES}")
target_link_libraries(knative_native endpoint faasm emulator scheduler)

# Add an executable to run it locally
add_definitions("-DKNATIVE_NATIVE=1")
add_executable(knative_native_runner knative_native_runner.cpp)
target_link_libraries(knative_native_runner knative_native)
