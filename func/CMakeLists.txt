cmake_minimum_required(VERSION 3.8.0)
project(faasm-func)

set(CMAKE_CXX_STANDARD 17)

# Lambda-specific stuff
if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    add_definitions(-DAWS_LAMBDA=1)
else ()
    add_definitions(-DAWS_LAMBDA=0)
endif ()

# Knative native-specific stuff
if (FAASM_BUILD_TYPE STREQUAL "knative-native")
    add_definitions(-DKNATIVE_NATIVE=1)
else ()
    add_definitions(-DKNATIVE_NATIVE=0)
endif ()

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    message(STATUS "Detected wasm build (sysroot=${CMAKE_SYSROOT}")

    # Eigen definitions
    # add_definitions(-DEIGEN_VECTORIZE_SSE2=1)

    # NOTE: we have to set the max memory here but want it to be as close to the
    # max (4GB) as possible. If we set it to the max though, this input is ignored
    # and the function is set with no maximum memory size.
    # Therefore we set it to 4GB - 1 page, i.e.
    # max-memory = (4*1024*1024*1024) - (64*1024) = 4294901760.
    #
    # WARNING: the code may also set limits on the max memory size, so must be changed there too (IRModuleRegistry.h)
    #
    # This max memory limit can stop benchmarks working so make sure it's big and consider restricting
    # in a more specific manner for certain functions.
    SET(CMAKE_EXE_LINKER_FLAGS "${FAASM_EXE_LINKER_FLAGS} \
        -Xlinker --export=_faasm_zygote \
        -Xlinker --max-memory=4294901760 \
        -Wl,-z,stack-size=4194304 -Wl, \
    ")

    SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
endif ()


# Define functions for building
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # ----------------------
    # Wasm build
    # ----------------------

    function(faasm_func_c func_name file_name)
        add_executable(${func_name} ${file_name})
        target_link_libraries(${func_name} faasm dlmalloc)
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        add_executable(${func_name} ${file_name})
        target_link_libraries(${func_name} faasm dlmalloc)
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})

        # These are the linker flags required for functions that will also do dynamic linking.
        # We need to export all symbols to make them available to the dynamically loaded modules
        set_target_properties(${func_name} PROPERTIES
                LINK_FLAGS "-Xlinker --export-all -Xlinker --no-gc-sections -Xlinker --export-dynamic"
                )

    endfunction(faasm_dynamic_func)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    # ----------------------
    # AWS Lambda build
    # ----------------------
    function(faasm_func_c func_name file_name)
        # Don't support C functions on AWS Lambda
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})
        target_link_libraries(${func_name}-lambda lambda-func)

        # Add custom target to build Lambda zip file
        aws_lambda_package_target(${func_name}-lambda)
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})
    endfunction(faasm_dynamic_func)

elseif (FAASM_BUILD_TYPE STREQUAL "knative-native")
    # ----------------------
    # Knative native build
    # ----------------------
    function(faasm_func_c func_name file_name)
        add_executable(${func_name}-knative ${file_name})
        target_link_libraries(
                ${func_name}-knative
                faasm
                knative_native
        )
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        faasm_func_c(${func_name} ${file_name})
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})
    endfunction(faasm_dynamic_func)

else ()
    # ----------------------
    # Native build
    # ----------------------

    function(faasm_func_c func_name file_name)
        add_executable(${func_name} ${file_name})
        target_link_libraries(${func_name} faasm emulator dl)
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        faasm_func_c(${func_name} ${file_name})
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})
        target_link_libraries(${func_name} util)
    endfunction(faasm_dynamic_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(dynlink)
add_subdirectory(gene)
add_subdirectory(polybench)
add_subdirectory(sgd)

# Skip certain builds with some build types
if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
else ()
    add_subdirectory(python)
    add_subdirectory(tf)
endif ()