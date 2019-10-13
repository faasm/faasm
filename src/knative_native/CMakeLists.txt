include_directories(
    ${FAASM_INCLUDE_DIR}/knative_native
)

set(LIB_FILES
    ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeHandler.h
    ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeEndpoint.h
    KnativeNativeHandler.cpp
    KnativeNativeEndpoint.cpp
  )

faasm_private_lib(worker_knative_native_lib "${LIB_FILES}")

target_link_libraries(worker_knative_native_lib endpoint)

add_executable(worker_knative_native worker_knative_native.cpp)

target_link_libraries(worker_knative_native worker_knative_native_lib)
