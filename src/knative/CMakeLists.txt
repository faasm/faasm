include_directories(
        ${FAASM_INCLUDE_DIR}/knative
)

add_executable(worker-knative
        ${FAASM_INCLUDE_DIR}/knative/KnativeHandler.h
        ${FAASM_INCLUDE_DIR}/knative/KnativeEndpoint.h
        KnativeHandler.cpp
        KnativeEndpoint.cpp
        worker_knative.cpp
        )

target_link_libraries(worker-knative
        http
        worker_lib
        )
