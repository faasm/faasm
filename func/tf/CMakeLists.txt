set(FAASM_USER tf)

set(TF_DIR /usr/local/code/faasm/third-party/tensorflow/)
set(TF_LITE_DIR ${TF_DIR}/tensorflow/lite)
set(TF_LITE_DOWNLOAD_DIR ${TF_LITE_DIR}/tools/make/downloads)

# Include TF headers and 3rd party
include_directories(
        ${TF_DIR}
        ${TF_LITE_DIR}
        ${TF_LITE_DOWNLOAD_DIR}
        ${TF_LITE_DOWNLOAD_DIR}/eigen
        ${TF_LITE_DOWNLOAD_DIR}/absl
        ${TF_LITE_DOWNLOAD_DIR}/gemmlowp
        ${TF_LITE_DOWNLOAD_DIR}/neon_2_sse
        ${TF_LITE_DOWNLOAD_DIR}/farmhash/src
        ${TF_LITE_DOWNLOAD_DIR}/flatbuffers/include
)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    function(tf_func exec_name dir_path)
        faasm_func(${exec_name} ${dir_path})
        target_link_libraries(${exec_name}
                /usr/local/code/faasm/third-party/tensorflow/tensorflow/lite/tools/make/gen/wasm32-unknown-none_x86_64/lib/libtensorflow-lite.a)
    endfunction(tf_func)
else()
    function(tf_func exec_name dir_path)
        faasm_func(${exec_name} ${dir_path})
        target_link_libraries(${exec_name}
                /usr/local/code/faasm/third-party/tensorflow/tensorflow/lite/tools/make/gen/linux_x86_64/lib/libtensorflow-lite.a)
    endfunction(tf_func)
endif()

add_subdirectory(image)
