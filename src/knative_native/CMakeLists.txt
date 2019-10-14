include_directories(
        ${FAASM_INCLUDE_DIR}/knative_native
)

set(LIB_FILES
        ${FAASM_INCLUDE_DIR}/knative_native/knative_emulator.h
        ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeHandler.h
        ${FAASM_INCLUDE_DIR}/knative_native/KnativeNativeEndpoint.h
        knative_emulator.cpp
        KnativeNativeHandler.cpp
        KnativeNativeEndpoint.cpp
        )

faasm_private_lib(knative_native "${LIB_FILES}")

target_link_libraries(knative_native endpoint redis)
