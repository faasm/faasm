cmake_minimum_required(VERSION 3.0)
project(faasm-sgd)

# Include Faasm headers
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/../include/faasm
)

# Faasm vars
set(FAASM_USER sgd)
set(FAASM_HOST localhost)

if (CMAKE_BUILD_TYPE MATCHES "wasm")
    # ----------------------
    # WASM build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} -Wl,--allow-undefined)

        # Upload URL
        set(UPLOAD_URL http://${FAASM_HOST}:8001/f/${FAASM_USER}/${exec_name})
        # Path to build module
        set(EXECUTABLE_PATH ${CMAKE_BINARY_DIR}/${exec_name})
        # Use CURL to upload
        add_custom_command(TARGET ${exec_name} POST_BUILD
            COMMAND curl -X PUT ${UPLOAD_URL} -T ${EXECUTABLE_PATH}
        )
    endfunction(faasm_func)

else()
    # ----------------------
    # Native build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} sgd_native.cpp ${file_name})
        target_link_libraries(${exec_name} emulator)
    endfunction(faasm_func)

endif()

# Add faasm functions
faasm_func(sgd_barrier sgd_barrier.cpp)
faasm_func(sgd_begin sgd_begin.cpp)
faasm_func(sgd_epoch sgd_epoch.cpp)
faasm_func(sgd_step sgd_step.cpp)