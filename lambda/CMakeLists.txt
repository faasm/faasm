include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${FAASM_INCLUDE_DIR}/lambda
        ${FAASM_LIB_DIR}
)

add_library(lambda STATIC
        backend.cpp
        ${FAASM_INCLUDE_DIR}/lambda/backend.h
        )

set(PUBLIC_HEADERS
        ${FAASM_INCLUDE_DIR}/lambda/backend.h
        )

set_target_properties(lambda PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

target_link_libraries(
        lambda
        AWS::aws-lambda-runtime
)

set_target_properties(lambda PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")
