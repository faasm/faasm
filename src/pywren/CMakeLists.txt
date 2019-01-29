include_directories(
        ${FAASM_INCLUDE_DIR}/pywren
        ${FAASM_WAVM_SOURCE_DIR}
)

add_library(pywren_lib
        handler.cpp
        ${FAASM_INCLUDE_DIR}/pywren/pywren.h
        )

target_link_libraries(pywren_lib aws worker_lib)

add_executable(pywren listener.cpp)
target_link_libraries(pywren pywren_lib)
