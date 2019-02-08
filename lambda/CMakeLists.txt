include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${FAASM_LIB_DIR}
)

add_library(lambda STATIC
        backend.cpp
        lambda.h
        backend.h
        )

set(PUBLIC_HEADERS
        lambda.h
        backend.h
        )

set_target_properties(lambda PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries(
        lambda
        faasm
        AWS::aws-lambda-runtime
)

set_target_properties(lambda PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
