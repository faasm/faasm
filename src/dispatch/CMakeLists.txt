include_directories(${FAASM_INCLUDE_DIR}/dispatch)

add_executable(dispatch-lambda dispatch_lambda.cpp)

target_link_libraries(dispatch-lambda lambda scheduler)

aws_lambda_package_target(dispatch-lambda)
