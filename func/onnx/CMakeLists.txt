set(FAASM_USER onnx)

# For some reason the code needs to link directly to onnxruntime_c_api.h with
# no directory structure, so need to add extra include dir
if (FAASM_BUILD_TYPE STREQUAL "wasm")
    include_directories(
            /usr/local/faasm/llvm-sysroot/include/onnxruntime
            /usr/local/faasm/llvm-sysroot/include/onnxruntime/core/session
    )

    set(EXTRA_LIBS onnxruntime png)
else ()
    include_directories(
            /usr/local/include/onnxruntime
            /usr/local/include/onnxruntime/core/session
    )

    set(EXTRA_LIBS /usr/local/lib/libonnxruntime.so.0.5.0 png)
endif ()

faasm_func_c(candy candy.c)
target_link_libraries(candy ${EXTRA_LIBS})
