include_directories(${FAASM_INCLUDE_DIR}/redis)

set(LIB_FILES
        Redis.cpp
        ${FAASM_INCLUDE_DIR}/redis/Redis.h
    )

faasm_private_lib(redis ${LIB_FILES})

target_link_libraries(redis util pthread hiredis)

if (FAASM_AWS_SUPPORT)
    add_executable(redis-lambda redis_lambda.cpp)
    target_link_libraries(redis-lambda
            redis
            aws
            AWS::aws-lambda-runtime
            )
    aws_lambda_package_target(redis-lambda)
endif()