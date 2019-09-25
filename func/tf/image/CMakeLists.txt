add_library(tf_img_lib
        bitmap_helpers.cc
        get_top_n.h
        get_top_n_impl.h
        label_image.h
)

add_definitions(-DWASM_PROF=1)

tf_func(image label_image.cc)
target_link_libraries(image tf_img_lib)