include_directories(${FAASM_INCLUDE_DIR}/codegen)

add_executable(codegen-lambda codegen_lambda.cpp)
target_link_libraries(codegen-lambda lambda)
aws_lambda_package_target(codegen-lambda)
