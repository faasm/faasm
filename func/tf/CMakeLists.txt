set(FAASM_USER tf)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(TF_DIR /usr/local/code/faasm/third-party/tensorflow/)
    set(TF_LITE_DIR ${TF_DIR}/tensorflow/lite)
    set(TF_LITE_DOWNLOAD_DIR ${TF_LITE_DIR}/tools/make/downloads)

    # Include TF headers
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
endif()

function(tf_func exec_name dir_path)
    set(TF_LIB "/usr/local/code/faasm/third-party/tensorflow/tensorflow/lite/tools/make/gen/wasm32-unknown-none_/lib/libtensorflow-lite.a")

    faasm_func(tf_${exec_name} ${dir_path})
    target_link_libraries(tf_${exec_name} ${TF_LIB})
endfunction(tf_func)

tf_func(minimal minimal.cpp)
