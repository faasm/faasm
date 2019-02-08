include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${FAASM_LIB_DIR}
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