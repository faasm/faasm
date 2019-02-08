cmake_minimum_required(VERSION 3.8.0)
project(lambda)

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
        lambda
        faasm
        AWS::aws-lambda-runtime
)

set_target_properties(lambda PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

#install(TARGETS lambda
#        ARCHIVE DESTINATION lib
#        LIBRARY DESTINATION lib
#        RUNTIME DESTINATION bin
#        PUBLIC_HEADER DESTINATION include/lambda
#        )