cmake_minimum_required(VERSION 3.0.0)

# Remove any host-specific flags
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_STANDARD 17)

# Tools
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_C_COMPILER "/usr/bin/clang-8")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-8")
set(CMAKE_AR "/usr/bin/llvm-ar-8")
set(CMAKE_RANLIB "/usr/bin/llvm-ranlib-8")
set(CMAKE_LINKER "/usr/bin/wasm-ld-8")

# Override compiler checks. Without this it'll fail trying to run wasm compilation
# on the test files
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# Compiler flags
# NOTE - these need to be explicitly set by the individual projects
set(WASM_SYSROOT "$ENV{HOME}/faasm/sysroot")
set(WASM_COMPILER_FLAGS "\
    -fvisibility=default \
    --sysroot=${WASM_SYSROOT} \
    --target=wasm32 \
")

# Avoid including any system files or libraries
# (should be compiling against the wasm-specific versions)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Musl header files
include_directories(${WASM_SYSROOT}/include)

# Linker
set(WASM_LINKER_FLAGS "--export-dynamic \
    --import-memory \
")

# Linker
set(CMAKE_C_LINK_FLAGS ${WASM_LINKER_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${WASM_LINKER_FLAGS})

SET(CMAKE_SHARED_LINKER_FLAGS "${WASM_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${WASM_LINKER_FLAGS}")

# Note, the CMAKE_XX_LINK_EXECUTABLE must specify the whole command, not just the executable
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Force suffixes
SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
SET(CMAKE_SHARED_MODULE_SUFFIX ".wasm")

# Definitions
add_definitions(-DWASM_BUILD=1)

