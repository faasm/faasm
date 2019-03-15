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
if (FAASM_BUILD_TYPE STREQUAL "lambda-func")
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

    # Couple of useful examples:
    # https://github.com/dgym/cpython-emscripten/blob/master/examples/03-python-app/Makefile
    # https://github.com/iodide-project/pyodide/blob/master/Makefile
    set(EM_OTHER_FLAGS "
    -s TOTAL_MEMORY=268435456 \
    -s MAIN_MODULE=1 \
    -s MINIMAL_RUNTIME=1 \
    -s FILESYSTEM=0 \
    -s USES_DYNAMIC_ALLOC=0 \
    -s EMULATED_FUNCTION_POINTERS=1 \
    ")

    # Emscripten-specific linker options
    # It's absolutely *crucial* to make sure these are actually getting used
    # To triple check you can enter some invalid values and see if stuff breaks
    # TODO - is there a way to avoid ignoring undefined symbols by respecting the libfaasm.imports file?
    set(EM_LINKER_FLAGS "-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
    -s ASSERTIONS=2 \
    -s EMULATE_FUNCTION_POINTER_CASTS=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s WASM=1 \
    -s EXPORTED_FUNCTIONS='[\"_main\", \"_PyRun_SimpleString\"]' \
    -s SIDE_MODULE=1 \
    -s \"BINARYEN_METHOD='native-wasm'\" \
    --memory-init-file 0 \
    ")

    set(CMAKE_EXE_LINKER_FLAGS "${EM_LINKER_FLAGS}")

    # Avoid compiling javascript
    set(CMAKE_REQUIRED_FLAGS "WASM=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

    # Link to custom-build faasm lib
    link_directories(/usr/local/code/faasm/lib/embuild/)
    set(FAASM_LIB libfaasm.a)
else ()
    set(FAASM_LIB faasm)
endif ()


# Define functions for building
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    # ----------------------
    # WASM build
    # ----------------------
    add_definitions(-DWASM_BUILD=1)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -fvisibility=hidden")

    function(faasm_func exec_name file_name)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

        add_executable(${exec_name} ${file_name})

        set_target_properties(${exec_name} PROPERTIES OUTPUT_NAME ${exec_name}.wasm)

        target_link_libraries(${exec_name} ${FAASM_LIB})
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
        target_link_libraries(${exec_name} ${FAASM_LIB} emulator)
    endfunction(faasm_func)

endif ()

add_subdirectory(errors)
add_subdirectory(demo)
add_subdirectory(sgd)

# Exclude python stuff in normal wasm build or lambda native
if (FAASM_BUILD_TYPE STREQUAL "wasm")
elseif (FAASM_BUILD_TYPE STREQUAL "lambda-func")
else ()
    add_subdirectory(python)
endif ()