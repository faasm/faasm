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
if (${EMSCRIPTEN})
    # Emscripten doesn't seem to look in normal system includes so we explicitly have to tell it to
    # include_directories(/usr/local/include/)

    message(STATUS "Detected emscripten build")

    # --- Emscripten python options ---
    #
    # These should match the main pyodide Makefile
    # https://github.com/iodide-project/pyodide/blob/master/Makefile
    #
    # We do not set MAIN_MODULE here as we are building pure WebAssembly
    set(EM_LINKER_FLAGS "-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
            -s MODULARIZE=1 \
            -s \"BINARYEN_METHOD='native-wasm'\" \
            -s \"BINARYEN_TRAP_MODE='clamp'\" \
            -s ALLOW_MEMORY_GROWTH=1 \
            -s LINKABLE=1 \
            -s EXPORT_ALL=1 \
            -s LZ4=1 \
    ")

    # Link to emscripten libfaasm
    link_directories(/usr/local/code/faasm/lib/emscripten_lib_build/)
    set(FAASM_LIB libfaasm.a)

    set(CMAKE_EXE_LINKER_FLAGS "${EM_LINKER_FLAGS}")
else ()
    set(FAASM_LIB faasm)
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

        target_link_libraries(${exec_name} ${FAASM_LIB})
        #        target_link_libraries(${exec_name} ${WASM_SYSROOT}/lib/crt1.o c c++ c++abi ${FAASM_LIB})
    endfunction(faasm_func)

elseif (${EMSCRIPTEN})
    # ----------------------
    # Emscripten build
    # ----------------------

    function(faasm_func exec_name file_name)
        add_executable(${exec_name} ${file_name})

        target_link_libraries(${exec_name} ${FAASM_LIB})
    endfunction(faasm_func)

elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
    # ----------------------
    # AWS Lambda build
    # ----------------------
    function(faasm_func func_name file_name)
        add_executable(${func_name}-lambda ${file_name})

        # Link with Faasm library
        target_link_libraries(${func_name}-lambda lambda-func)

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
        target_link_libraries(${exec_name} ${FAASM_LIB} emulator dl)
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