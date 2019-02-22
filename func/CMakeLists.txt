cmake_minimum_required(VERSION 3.8.0)
project(faasm-func)

set(CMAKE_CXX_STANDARD 17)

set(FAASM_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)

include_directories(${FAASM_LIB_DIR})

# Faasm host for uploads
if (DEFINED ENV{FAASM_HOST})
    set(FAASM_HOST $ENV{FAASM_HOST})
else ()
    set(FAASM_HOST localhost)
endif ()

# Modify inclusions if building for lambda
if (FAASM_BUILD_TYPE STREQUAL "lambda")
    add_definitions(-DAWS_LAMBDA=1)

    include_directories(${FAASM_LIB_DIR} ${FAASM_LAMBDA_LIB_DIR})
else ()
    add_definitions(-DAWS_LAMBDA=0)
endif ()

# Emscripten fiddles
if (${EMSCRIPTEN})
    message(STATUS "Detected Emscripten build")

    # Emscripten doesn't seem to look in normal system includes so we explicitly have to tell it to
    include_directories(/usr/local/include/)

    # Ignore undefined symbols
    # TODO - there must be a way to respect the libfaasm.imports file
    set(CMAKE_EXE_LINKER_FLAGS  "-s ERROR_ON_UNDEFINED_SYMBOLS=0")

    # Avoid compiling javascript
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

    # Link to custom-build faasm lib
    link_directories(/usr/local/code/faasm/lib/embuild/)
    set(FAASM_LIB libfaasm.a)
else()
    set(FAASM_LIB faasm)
endif()


# Define functions for building
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # ----------------------
    # WASM build
    # ----------------------
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -fvisibility=hidden")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=wasm32-unknown-unknown-wasm")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=/toolchain/sysroot")

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})

        target_link_libraries(${exec_name} ${FAASM_LIB})
    endfunction(faasm_func)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda")
    # ----------------------
    # AWS Lambda build
    # ----------------------

    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})

        # Link with Faasm lambda library
        target_link_libraries(${func_name}-lambda faasm lambda)

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
        # Using emulator (faasm-like)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm emulator)

        # Using lambda back-end (don't do for python functions)
        if(exec_name MATCHES "py_*")
        else()
            add_executable(${exec_name}-lambda ${file_name})
            target_link_libraries(${exec_name}-lambda faasm lambda)
        endif()
    endfunction(faasm_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(sgd)
add_subdirectory(python)
