cmake_minimum_required(VERSION 3.8.0)
project(faasm-func)

set(CMAKE_CXX_STANDARD 17)

if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    add_definitions(-DAWS_LAMBDA=1)

    # Add faasm lambda headers
    include_directories(${FAASM_LAMBDA_LIB_DIR})
else ()
    add_definitions(-DAWS_LAMBDA=0)
endif ()

# Emscripten fiddles
if (FAASM_BUILD_TYPE STREQUAL "emscripten")
    message(STATUS "Detected emscripten build")

    SET(CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-entry -Xlinker --export=main")

    SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
endif ()


# Define functions for building
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(CMAKE_CXX_FLAGS "${STANDARD_EXE_FLAGS}")
    set(CMAKE_C_FLAGS "${STANDARD_EXE_FLAGS}")

    # ----------------------
    # WASM build
    # ----------------------
    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        set_target_properties(${exec_name} PROPERTIES OUTPUT_NAME ${exec_name}.wasm)
        target_link_libraries(${exec_name} faasm)
    endfunction(faasm_func)

elseif (FAASM_BUILD_TYPE STREQUAL "emscripten")
    # ----------------------
    # Emscripten build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm)
    endfunction(faasm_func)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    # ----------------------
    # AWS Lambda build
    # ----------------------
    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})
        target_link_libraries(${func_name}-lambda lambda-func)

        # Add custom target to build Lambda zip file
        aws_lambda_package_target(${func_name}-lambda)

    endfunction(faasm_func)

else ()
    # ----------------------
    # Native build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm emulator dl)
    endfunction(faasm_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(sgd)

# Add python build in everything but the lambda funcs
if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
else ()
    add_subdirectory(python)
endif ()