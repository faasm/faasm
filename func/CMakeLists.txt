cmake_minimum_required(VERSION 3.0)
project(faasm-func)

set(FAASM_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)

set(FAASM_HOST localhost)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -fvisibility=hidden")

include_directories(${FAASM_LIB_DIR})

# Define functions for building
if (CMAKE_BUILD_TYPE MATCHES "wasm")
    # ----------------------
    # WASM build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm)

        # Upload URL
        set(UPLOAD_URL http://${FAASM_HOST}:8001/f/${FAASM_USER}/${exec_name})
        # Path to build module
        set(EXECUTABLE_PATH ${CMAKE_BINARY_DIR}/${FAASM_USER}/${exec_name})
        # Use CURL to upload
        add_custom_command(TARGET ${exec_name} POST_BUILD
                COMMAND curl -X PUT ${UPLOAD_URL} -T ${EXECUTABLE_PATH}
                )
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
