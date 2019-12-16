set(FAASM_USER tf)

set(TF_DIR /usr/local/code/faasm/third-party/tensorflow/)
set(TF_LITE_DIR ${TF_DIR}/tensorflow/lite)
set(TF_LITE_DOWNLOAD_DIR ${TF_LITE_DIR}/tools/make/downloads)
set(TF_BUILD_DIR ${TF_LITE_DIR}/tools/make/gen/)

# Turn on WASM profiling
add_definitions(-DWASM_PROF=1)

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

# Set where the library has been built
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    set(TF_LIB ${TF_BUILD_DIR}/wasm32-unknown-none_x86_64/lib/libtensorflow-lite.a)
else()
    set(TF_LIB ${TF_BUILD_DIR}/linux_x86_64/lib/libtensorflow-lite.a)
endif()

# Build shared library to use with function
add_library(tf_img_lib
        bitmap_helpers.cc
        get_top_n.h
        get_top_n_impl.h
        image.h
        )

target_link_libraries(tf_img_lib ${TF_LIB})

# Add the actual Faasm function
faasm_func(image image.cc)

# Link with library
if(FAASM_BUILD_TYPE STREQUAL "knative-native")
    target_link_libraries(image-knative tf_img_lib)
else()
    target_link_libraries(image tf_img_lib)
endif()
