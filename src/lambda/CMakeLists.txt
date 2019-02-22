include_directories(
        ${FAASM_INCLUDE_DIR}/lambda
)

add_library(lambda STATIC
        backend.cpp
        ${FAASM_INCLUDE_DIR}/lambda/backend.h
        )

target_link_libraries(
        lambda
        aws
        AWS::aws-lambda-runtime
)
