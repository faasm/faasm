cmake_minimum_required(VERSION 3.8.2) # target_include_directories

set(SGX_FOUND "NO")

set(SGX_SDK_PATH "/opt/intel/sgxsdk")
find_path(SGX_INCLUDE_DIR sgx.h "${SGX_SDK_PATH}/include" NO_DEFAULT_PATH)
find_path(SGX_LIBRARY_DIR libsgx_urts.so "${SGX_SDK_PATH}/lib64" NO_DEFAULT_PATH)

if(SGX_INCLUDE_DIR AND SGX_LIBRARY_DIR)
    set(SGX_FOUND "YES")
    message(STATUS "Found Intel SGX SDK.")
else()
    message(STATUS "Intel SGX SDK not found")
endif()
