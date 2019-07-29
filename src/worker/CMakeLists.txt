include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

set(LIB_HEADERS
        ${FAASM_INCLUDE_DIR}/worker/WorkerThread.h
        ${FAASM_INCLUDE_DIR}/worker/WorkerThreadPool.h
        )

set(LIB_FILES
        WorkerThread.cpp
        WorkerThreadPool.cpp
        ${LIB_HEADERS}
        )

add_library(worker_lib "${LIB_FILES}")
target_link_libraries(worker_lib wasm scheduler system)

# Bare metal worker
add_executable(worker worker_main.cpp)
target_link_libraries(worker worker_lib)

# Lambda worker
add_executable(worker-lambda worker_lambda.cpp)
target_link_libraries(worker-lambda
        aws
        AWS::aws-lambda-runtime
        worker_lib
        )
aws_lambda_package_target(worker-lambda)

# Knative worker
add_executable(worker-knative
        ${FAASM_INCLUDE_DIR}/worker/KnativeHandler.h
        ${FAASM_INCLUDE_DIR}/worker/KnativeEndpoint.h
        KnativeHandler.cpp
        KnativeEndpoint.cpp
        worker_knative.cpp)
target_link_libraries(worker-knative
        http
        worker_lib
        )
