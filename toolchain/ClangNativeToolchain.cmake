cmake_minimum_required(VERSION 3.4.0)

set(INSTALL_DIR /usr/bin)
set(EXE_SUFFIX "")

set(UNIX 1)

set(CMAKE_C_COMPILER ${INSTALL_DIR}/clang)
set(CMAKE_CXX_COMPILER ${INSTALL_DIR}/clang++)

set(CMAKE_AR ${INSTALL_DIR}/llvm-ar CACHE STRING "native build")
set(CMAKE_NM ${INSTALL_DIR}/llvm-nm CACHE STRING "native build")
set(CMAKE_RANLIB ${INSTALL_DIR}/llvm-ranlib CACHE STRING "native build")
set(CMAKE_LD ${INSTALL_DIR}/lld CACHE STRING "native build")
set(CMAKE_LDSHARED ${INSTALL_DIR}/lld CACHE STRING "native build")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3" CACHE STRING "native build")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3" CACHE STRING "native build")

set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)
