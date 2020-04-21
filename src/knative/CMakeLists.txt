include_directories(
        ${FAASM_INCLUDE_DIR}/knative
)

set(LIB_FILES
        ${FAASM_INCLUDE_DIR}/knative/KnativeHandler.h
        ${FAASM_INCLUDE_DIR}/knative/KnativeEndpoint.h
        KnativeHandler.cpp
        KnativeEndpoint.cpp
        )

faasm_private_lib(worker_knative_lib "${LIB_FILES}")

target_link_libraries(worker_knative_lib endpoint worker_lib)

add_executable(worker_knative worker_knative.cpp)

target_link_libraries(worker_knative worker_knative_lib)
