cmake_minimum_required(VERSION 3.4.0)

set(TOOLCHAIN_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(INSTALL_DIR ${TOOLCHAIN_ROOT}/install/bin)

set(FAASM_SYSROOT /usr/local/faasm/llvm-sysroot)

set(EXE_SUFFIX "")

set(UNIX 1)

set(CMAKE_C_COMPILER ${INSTALL_DIR}/clang)
set(CMAKE_CXX_COMPILER ${INSTALL_DIR}/clang++)

set(CMAKE_AR ${INSTALL_DIR}/llvm-ar CACHE STRING "faasm-sdk build")
set(CMAKE_NM ${INSTALL_DIR}/llvm-nm CACHE STRING "faasm-sdk build")
set(CMAKE_RANLIB ${INSTALL_DIR}/llvm-ranlib CACHE STRING "faasm-sdk build")
set(CMAKE_LD ${INSTALL_DIR}/wasm-ld CACHE STRING "faasm-sdk build")
set(CMAKE_LDSHARED ${INSTALL_DIR}/wasm-ld CACHE STRING "faasm-sdk build")
set(CMAKE_C_COMPILER_TARGET wasm32 CACHE STRING "faasm-sdk build")
set(CMAKE_CXX_COMPILER_TARGET wasm32 CACHE STRING "faasm-sdk build")

set(CMAKE_DL_LIBS "")

set(FAASM_COMPILER_FLAGS "--sysroot=${FAASM_SYSROOT} -I${FAASM_SYSROOT}/include/eigen3")

set(CMAKE_SYSROOT ${FAASM_SYSROOT} CACHE STRING "faasm-sdk build")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FAASM_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FAASM_COMPILER_FLAGS}")

# Note that system name and processor here are crucial
# Setting system name automatically switches on cross-compiling
set(CMAKE_SYSTEM_NAME Wasm)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)
