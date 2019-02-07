cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)
project(encoder LANGUAGES CXX)

# Note that here we force the install prefix as it must be done in the context of
# the custom build (to avoid pulling in the standard system files)
SET(CMAKE_INSTALL_PREFIX ~/faasm/lambda CACHE PATH override FORCE)

find_package(aws-lambda-runtime)
find_package(AWSSDK REQUIRED COMPONENTS s3 sqs)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE AWS::aws-lambda-runtime ${AWSSDK_LINK_LIBRARIES})

target_compile_options(${PROJECT_NAME} PRIVATE
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wshadow"
        "-Wno-sign-conversion")

target_compile_features(${PROJECT_NAME} PRIVATE "cxx_std_11")

aws_lambda_package_target(${PROJECT_NAME})

