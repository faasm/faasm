set(FAASM_USER onnx)

if (FAASM_BUILD_TYPE STREQUAL "wasm")
else ()
    set(ONNX_LIBS /usr/local/code/faasm/onnxruntime/build/native/Debug/libonnxruntime.a)
    include_directories("/usr/local/code/faasm/onnxruntime/include")
endif ()

faasm_func_c(candy candy.c)

target_link_libraries(candy ${ONNX_LIBS})
