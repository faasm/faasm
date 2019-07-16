include_directories(${FAASM_INCLUDE_DIR}/aws)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/aws/*.h")

set(LIB_FILES
    aws.cpp
    LambdaWrapper.cpp
    S3Wrapper.cpp
    SQSWrapper.cpp
    ${HEADERS}
)

faasm_private_lib(aws "${LIB_FILES}")

target_link_libraries(
        aws
        util
        ${AWSSDK_LINK_LIBRARIES}
)
