include_directories(
        ${FAASM_INCLUDE_DIR}/state
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/state/*.h")

set(LIB_FILES
        State.cpp
        UserState.cpp
        StateKeyValue.cpp
        ${HEADERS}
    )

faasm_private_lib(state "${LIB_FILES}")
target_link_libraries(state proto redis)

if(FAASM_AWS_SUPPORT)
    add_executable(state-lambda state_lambda.cpp)
    target_link_libraries(state-lambda
            redis
            aws
            AWS::aws-lambda-runtime
            )
    aws_lambda_package_target(state-lambda)
endif()