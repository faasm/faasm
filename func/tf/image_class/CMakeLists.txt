add_library(tf_img_lib
        bitmap_helpers.cc
        bitmap_helpers.h
        bitmap_helpers_impl.h
        get_top_n.h
        get_top_n_impl.h
        label_image.h
)

tf_func(image label_image.cc)

target_link_libraries(tf_image tf_img_lib)