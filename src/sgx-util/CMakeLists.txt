include_directories(${FAASM_INCLUDE_DIR}/sgx-util)

find_package(RapidJSON)

file(GLOB HEADERS "${FAASM_INCLUDE_DIR}/sgx-util/*.h")

set(LIB_FILES
        base64.cpp
        hash.cpp
)

faasm_private_lib(sgx-util "${LIB_FILES}")

target_link_libraries(
        sgx-util
        ssl
)
