include_directories(${FAASM_INCLUDE_DIR}/aws)

find_package(AWSSDK REQUIRED COMPONENTS s3 sqs)

add_library(aws STATIC
        aws.cpp
        ${FAASM_INCLUDE_DIR}/aws/aws.h
)

target_link_libraries(
        aws
        util
        ${AWSSDK_LINK_LIBRARIES}
)
