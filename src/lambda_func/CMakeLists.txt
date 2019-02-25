include_directories(
        ${FAASM_INCLUDE_DIR}/lambda_func
)

add_library(lambda-func STATIC
        interface.cpp
        ${FAASM_INCLUDE_DIR}/lambda_func/interface.h
        )

target_link_libraries(
        lambda-func
        faasm
        aws
        redis
        AWS::aws-lambda-runtime
)
