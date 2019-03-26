cmake_minimum_required(VERSION 3.0.0)

# Remove any host-specific flags
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE on)

# LLVM Tools
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
set(STANDARD_EXE_FLAGS "\
    -fvisibility=default \
    --target=wasm32 \
    --sysroot=${WASM_SYSROOT} \
    ")

# Note, these need to be added explicitly for shared libs
# CMake will claim the linker args aren't used
# We need shared objects to import memory and table from their environment,
# and to export their functions
set(LIB_COMPILER_FLAGS "${STANDARD_EXE_FLAGS} \
    -Xlinker --import-memory \
    -Xlinker --import-table \
    -Xlinker --shared \
")

# For executables we want to use with dynamic linking, they must export
# all their symbols
set(DYNLINK_EXE_FLAGS "${STANDARD_EXE_FLAGS}  \
    -Xlinker --export-all \
")

# Force suffixes
SET(CMAKE_EXECUTABLE_SUFFIX ".wasm")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")
SET(CMAKE_SHARED_MODULE_SUFFIX ".wasm")

# Definitions
add_definitions(-DWASM_BUILD=1)

# Install location
SET(CMAKE_INSTALL_PREFIX ${WASM_SYSROOT} CACHE STRING "" FORCE)