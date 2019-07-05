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

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    message(STATUS "Detected wasm build (sysroot=${CMAKE_SYSROOT}")

    add_definitions(-DWASM_BUILD=1)

    # -------- NOTE ----------
    # The stack-first here is really important to help detect stack overflow
    # issues. Without it the stack will overflow into the global data.
    # stack-size is also crucial to bigger functions not messing up
    SET(CMAKE_EXE_LINKER_FLAGS "\
        -Xlinker --no-entry \
        -Xlinker --export=main \
        -Xlinker --max-memory=1073741824 \
        -Xlinker --stack-first \
        -Xlinker --threads \
        -Wl,-z,stack-size=4194304 -Wl, \
    ")

    # This shouldn't be needed (.imports files alongside linked .a files should do the trick)
    #  -Xlinker --allow-undefined-file=${CMAKE_SYSROOT}/lib/libfaasm.imports \

    SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
endif ()


# Define functions for building
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # ----------------------
    # Wasm build
    # ----------------------

    function(faasm_func_c exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasmc dlmalloc)
    endfunction(faasm_func_c)

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm dlmalloc)
    endfunction(faasm_func)

    function(faasm_dynamic_func exec_name file_name)
        faasm_func(${exec_name} ${file_name})

        # These are the linker flags required for functions that will also do dynamic linking.
        # We need to export all symbols to make them available to the dynamically loaded modules
        set_target_properties(${exec_name} PROPERTIES
                LINK_FLAGS "-Xlinker --export-all -Xlinker --no-gc-sections -Xlinker --export-dynamic"
        )

    endfunction(faasm_dynamic_func)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    # ----------------------
    # AWS Lambda build
    # ----------------------
    function(faasm_func_c exec_name file_name)
        # Don't support C functions on AWS Lambda
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})
        target_link_libraries(${func_name}-lambda lambda-func)

        # Add custom target to build Lambda zip file
        aws_lambda_package_target(${func_name}-lambda)
    endfunction(faasm_func)

    function(faasm_dynamic_func exec_name file_name)
        faasm_func(${exec_name} ${file_name})
    endfunction(faasm_dynamic_func)

else ()
    # ----------------------
    # Native build
    # ----------------------
    function(faasm_func_c exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasmc emulatorc dl)
    endfunction(faasm_func_c)

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})
        target_link_libraries(${exec_name} faasm emulator dl)
    endfunction(faasm_func)

    function(faasm_dynamic_func exec_name file_name)
        faasm_func(${exec_name} ${file_name})
        target_link_libraries(${exec_name} util)
    endfunction(faasm_dynamic_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(sgd)
add_subdirectory(dynlink)
add_subdirectory(polybench)

# Add python build in everything but the lambda funcs
if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
else ()
    add_subdirectory(python)
endif ()