include_directories(${FAASM_INCLUDE_DIR}/dispatch)

add_executable(dispatch-lambda dispatch_lambda.cpp)

target_link_libraries(dispatch-lambda
        aws
        AWS::aws-lambda-runtime
        scheduler
    )

if(FAASM_AWS_SUPPORT)
    aws_lambda_package_target(dispatch-lambda)
endif()