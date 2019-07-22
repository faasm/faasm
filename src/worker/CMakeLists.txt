include_directories(
        ${FAASM_INCLUDE_DIR}/worker
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/worker/*.h")

set(LIB_FILES
        WorkerThread.cpp
        WorkerThreadPool.cpp
        ${HEADERS}
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