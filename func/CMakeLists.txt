cmake_minimum_required(VERSION 3.8.0)
project(faasm-func)

set(FAASM_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)

if (DEFINED ENV{FAASM_HOST})
    set(FAASM_HOST $ENV{FAASM_HOST})
else ()
    set(FAASM_HOST localhost)
endif ()

include_directories(${FAASM_LIB_DIR})

if (CMAKE_BUILD_TYPE MATCHES "lambda")
    add_definitions(-DAWS_LAMBDA=1)
else ()
    add_definitions(-DAWS_LAMBDA=0)
endif ()

# Define functions for building
if (CMAKE_BUILD_TYPE MATCHES "wasm")
    # ----------------------
    # WASM build
    # ----------------------

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -fvisibility=hidden")

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm)

        # Upload URL
        set(UPLOAD_URL http://${FAASM_HOST}:8002/f/${FAASM_USER}/${exec_name})
        # Path to build module
        set(EXECUTABLE_PATH ${CMAKE_BINARY_DIR}/${FAASM_USER}/${exec_name})
        # Use CURL to upload
        add_custom_command(TARGET ${exec_name} POST_BUILD
                COMMAND curl -X PUT ${UPLOAD_URL} -T ${EXECUTABLE_PATH}
                )
    endfunction(faasm_func)

elseif (CMAKE_BUILD_TYPE MATCHES "lambda")
    # ----------------------
    # AWS Lambda build
    # ----------------------

    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})

        # Link with Faasm lambda library
        target_link_libraries(${func_name}-lambda lambda faasm)

#        target_compile_options(
#                ${func_name}-lambda PRIVATE
#                "-Wall"
#                "-Wextra"
#                "-Wconversion"
#                "-Wshadow"
#                "-Wno-sign-conversion"
#        )

        #target_compile_features(${PROJECT_NAME} PRIVATE "cxx_std_11")

        # Add custom target to build Lambda zip file
        aws_lambda_package_target(${func_name}-lambda)

    endfunction(faasm_func)

else ()
    # ----------------------
    # Native build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm emulator)
    endfunction(faasm_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(sgd)
