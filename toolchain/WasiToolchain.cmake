cmake_minimum_required(VERSION 3.4.0)

# -----------------------------------------
# A useful reference for this file is the wasi-sdk equivalent:
# https://github.com/WebAssembly/wasi-sdk/blob/master/wasi-sdk.cmake
# If anything breaks it's useful to compare the two.
#
# BUT there are lots of crucial modifications in here and it's
# used all over the place in this project, so be **very** careful
# if changing it.
# -----------------------------------------

set(INSTALL_DIR /usr/local/faasm/toolchain/bin)
set(FAASM_SYSROOT /usr/local/faasm/llvm-sysroot)

set(EXE_SUFFIX "")

set(UNIX 1)

# Note that system name and processor here are crucial
# Setting system name automatically switches on cross-compiling
set(WASM_TRIPLE wasm32-wasi)
set(CMAKE_SYSTEM_NAME Wasm)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)
set(CMAKE_C_COMPILER_TARGET ${WASM_TRIPLE} CACHE STRING "faasm build")
set(CMAKE_CXX_COMPILER_TARGET ${WASM_TRIPLE} CACHE STRING "faasm build")

# Specify LLVM toolchain
set(CMAKE_C_COMPILER ${INSTALL_DIR}/clang)
set(CMAKE_CXX_COMPILER ${INSTALL_DIR}/clang++)
set(CMAKE_AR ${INSTALL_DIR}/llvm-ar CACHE STRING "faasm build")
set(CMAKE_NM ${INSTALL_DIR}/llvm-nm CACHE STRING "faasm build")
set(CMAKE_RANLIB ${INSTALL_DIR}/llvm-ranlib CACHE STRING "faasm build")
#set(CMAKE_LD ${INSTALL_DIR}/wasm-ld CACHE STRING "faasm build")
#set(CMAKE_LDSHARED ${INSTALL_DIR}/wasm-ld CACHE STRING "faasm build")

set(CMAKE_DL_LIBS "")

# Explicitly disable eigen parallelisation
add_definitions(-DEIGEN_DONT_PARALLELIZE=1)

# Add definition for flagging Faasm
add_definitions(-D__faasm)

# Note: see Clang wasm-specific flags at https://clang.llvm.org/docs/ClangCommandLineReference.html#webassembly
# Note the optimisation level. We want to keep vectorization so stick with O3
# Also note that the optimisation is crucial to getting any kind of decent performance
set(FAASM_COMPILER_FLAGS "-O3 -msimd128 --sysroot=${FAASM_SYSROOT}")

set(CMAKE_SYSROOT ${FAASM_SYSROOT} CACHE STRING "faasm build")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FAASM_COMPILER_FLAGS}" CACHE STRING "faasm build")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FAASM_COMPILER_FLAGS}" CACHE STRING "faasm build")

# This is important to ensure the right search path
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)

# -------- NOTE ----------
# The stack-first here is really important to help detect stack overflow
# issues. Without it the stack will overflow into the global data.
# stack-size is also crucial to bigger functions not messing up

SET(FAASM_COMMON_LINKER_FLAGS "\
    -Xlinker --shared-memory \
    -Xlinker --stack-first \
    -Xlinker --no-check-features \
    -Xlinker --threads \
    -Xlinker --max-memory=4294901760 \
")

SET(FAASM_EXE_LINKER_FLAGS "${FAASM_COMMON_LINKER_FLAGS} \
    -Xlinker --no-entry \
    -Xlinker --export=main \
")

# Note, these get passed to llvm-ar for static libs, so 
# don't set for CMAKE_STATIC_LINKER_FLAGS
SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${FAASM_COMMON_LINKER_FLAGS} CACHE STRING "faasm build")
