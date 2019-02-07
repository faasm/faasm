cmake_minimum_required(VERSION 3.8.0)
project(lambda)

SET(CMAKE_INSTALL_PREFIX ~/faasm/lambda CACHE PATH override FORCE)

# AWS-specific set-up
find_package(aws-lambda-runtime)
find_package(AWSSDK REQUIRED COMPONENTS s3 sqs)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
)

set(PUBLIC_HEADERS
        lambda.h
        backend.h
        )

add_library(lambda STATIC
        backend.cpp
        ${PUBLIC_HEADERS}
        )

target_link_libraries(
        lambda PRIVATE
        faasm
        AWS::aws-lambda-runtime
        ${AWSSDK_LINK_LIBRARIES}
)

set_target_properties(faasm PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

