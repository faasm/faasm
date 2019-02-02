include_directories(${FAASM_INCLUDE_DIR}/aws)

find_package(AWSSDK REQUIRED COMPONENTS s3 sqs)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/aws/*.h")

add_library(aws STATIC
        aws.cpp
        S3Wrapper.cpp
        SQSWrapper.cpp
        ${HEADERS}
)

target_link_libraries(
        aws
        util
        ${AWSSDK_LINK_LIBRARIES}
)
