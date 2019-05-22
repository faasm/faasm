include_directories(${FAASM_INCLUDE_DIR}/redis)

add_library(
        redis STATIC
        Redis.cpp
        ${FAASM_INCLUDE_DIR}/redis/Redis.h
)

target_link_libraries(redis util pthread hiredis)

add_executable(redis-lambda redis_lambda.cpp)
target_link_libraries(redis-lambda
        redis
        aws
        AWS::aws-lambda-runtime
        )
aws_lambda_package_target(redis-lambda)