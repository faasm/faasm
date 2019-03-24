cmake_minimum_required(VERSION 3.0.0)

# Remove any host-specific flags
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_STANDARD 17)

# LLVM Tools
set(CMAKE_CROSSCOMPILING TRUE)
set(TOOLCHAIN_ROOT "$ENV{HOME}/faasm/toolchain")
set(CMAKE_C_COMPILER "${TOOLCHAIN_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}/bin/clang++")
set(CMAKE_LINKER "${TOOLCHAIN_ROOT}/bin/wasm-ld" CACHE FILEPATH "")

# For some reason these need to be CACHE variables
set(CMAKE_AR "${TOOLCHAIN_ROOT}/bin/llvm-ar" CACHE FILEPATH "")
set(CMAKE_RANLIB "${TOOLCHAIN_ROOT}/bin/llvm-ranlib" CACHE FILEPATH "")
set(CMAKE_NM "${TOOLCHAIN_ROOT}/bin/llvm-nm" CACHE FILEPATH "")

# Override compiler checks. Without this it'll fail trying to run wasm
# compilation on the test files
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

# Compiler flags
# NOTE: we don't want to use CMAKE_SYSROOT as this messes with absolute paths
set(WASM_SYSROOT "${TOOLCHAIN_ROOT}/sysroot")
set(WASM_COMPILER_FLAGS "\
    -fvisibility=default \
    --target=wasm32 \
    --sysroot=${WASM_SYSROOT} \
    ")

set(CMAKE_C_FLAGS ${WASM_COMPILER_FLAGS} CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS ${WASM_COMPILER_FLAGS} CACHE STRING "" FORCE)

# Avoid including any system files or libraries
# (should be compiling against the wasm-specific versions)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Include dirs/ lib dirs
include_directories(${WASM_SYSROOT}/include)
link_directories(${WASM_SYSROOT}/lib)

# Linker
set(WASM_LINKER_FLAGS "--export-dynamic \
    --import-memory \
    --no-entry \
")

# Linker
SET(CMAKE_STATIC_LINKER_FLAGS "${WASM_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${WASM_LINKER_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${WASM_LINKER_FLAGS}")

## Note, the CMAKE_XXX_LINK_EXECUTABLE must specify the whole command template, not just the executable
set(CMAKE_C_LINK_FLAGS ${WASM_LINKER_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${WASM_LINKER_FLAGS})
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Force suffixes
SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
SET(CMAKE_SHARED_MODULE_SUFFIX ".wasm")

# Definitions
add_definitions(-DWASM_BUILD=1)

# Install location
SET(CMAKE_INSTALL_PREFIX ${WASM_SYSROOT} CACHE STRING "" FORCE)