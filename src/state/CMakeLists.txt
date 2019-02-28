include_directories(
        ${FAASM_INCLUDE_DIR}/state
        ${FAASM_WAVM_SOURCE_DIR}
)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/state/*.h")

add_library(state STATIC
        State.cpp
        UserState.cpp
        StateKeyValue.cpp
        ${HEADERS}
        )

target_link_libraries(state proto redis)

# State Lambda function
add_executable(state-lambda state_lambda.cpp)
target_link_libraries(state-lambda redis lambda)
aws_lambda_package_target(state-lambda)
