include_directories(${FAASM_INCLUDE_DIR}/aws)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/aws/*.h")

add_library(aws STATIC
        aws.cpp
        LambdaWrapper.cpp
        S3Wrapper.cpp
        SQSWrapper.cpp
        ${HEADERS}
)

target_link_libraries(
        aws
        util
        ${AWSSDK_LINK_LIBRARIES}
)
