#include "onnxruntime/core/session/onnxruntime_c_api.h"
#include "onnxruntime/core/providers/cpu/cpu_provider_factory.h"
#include <stdio.h>
#include <assert.h>
#include <png.h>

#define ORT_ABORT_ON_ERROR(expr)                         \
  do {                                                   \
    OrtStatus* onnx_status = (expr);                     \
    if (onnx_status != NULL) {                           \
      const char* msg = OrtGetErrorMessage(onnx_status); \
      fprintf(stderr, "%s\n", msg);                      \
      OrtReleaseStatus(onnx_status);                     \
      abort();                                           \
    }                                                    \
  } while (0);

/**
 * convert input from HWC format to CHW format
 * \param input A single image. The byte array has length of 3*h*w
 * \param h image height
 * \param w image width
 * \param output A float array. should be freed by caller after use
 * \param output_count Array length of the `output` param
 */
static void hwc_to_chw(const png_byte* input, size_t h, size_t w, float** output, size_t* output_count) {
    size_t stride = h * w;
    *output_count = stride * 3;
    float* output_data = (float*)malloc(*output_count * sizeof(float));
    for (size_t i = 0; i != stride; ++i) {
        for (size_t c = 0; c != 3; ++c) {
            output_data[c * stride + i] = input[i * 3 + c];
        }
    }
    *output = output_data;
}

/**
 * convert input from CHW format to HWC format
 * \param input A single image. This float array has length of 3*h*w
 * \param h image height
 * \param w image width
 * \param output A byte array. should be freed by caller after use
 */
static void chw_to_hwc(const float* input, size_t h, size_t w, png_bytep* output) {
    size_t stride = h * w;
    png_bytep output_data = (png_bytep)malloc(stride * 3);
    for (int c = 0; c != 3; ++c) {
        size_t t = c * stride;
        for (size_t i = 0; i != stride; ++i) {
            float f = input[t + i];
            if (f < 0.f || f > 255.0f) f = 0;
            output_data[i * 3 + c] = (png_byte)f;
        }
    }
    *output = output_data;
}

/**
 * \param out should be freed by caller after use
 * \param output_count Array length of the `out` param
 */
static int read_png_file(const char* input_file, size_t* height, size_t* width, float** out, size_t* output_count) {
    png_image image; /* The control structure used by libpng */
    /* Initialize the 'png_image' structure. */
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&image, input_file) == 0) {
        return -1;
    }
    png_bytep buffer;
    image.format = PNG_FORMAT_BGR;
    size_t input_data_length = PNG_IMAGE_SIZE(image);
    if (input_data_length != 720 * 720 * 3) {
        printf("input_data_length:%zd\n", input_data_length);
        return -1;
    }
    buffer = (png_bytep)malloc(input_data_length);
    memset(buffer, 0, input_data_length);
    if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/, NULL /*colormap*/) == 0) {
        return -1;
    }
    hwc_to_chw(buffer, image.height, image.width, out, output_count);
    free(buffer);
    *width = image.width;
    *height = image.height;
    return 0;
}

/**
 * \param tensor should be a float tensor in [N,C,H,W] format
 */
static int write_tensor_to_png_file(OrtValue* tensor, const char* output_file) {
    struct OrtTensorTypeAndShapeInfo* shape_info;
    ORT_ABORT_ON_ERROR(OrtGetTensorTypeAndShape(tensor, &shape_info));
    size_t dim_count;
    ORT_ABORT_ON_ERROR(OrtGetDimensionsCount(shape_info, &dim_count));
    if (dim_count != 4) {
        printf("output tensor must have 4 dimensions");
        return -1;
    }
    int64_t dims[4];
    ORT_ABORT_ON_ERROR(OrtGetDimensions(shape_info, dims, sizeof(dims) / sizeof(dims[0])));
    if (dims[0] != 1 || dims[1] != 3) {
        printf("output tensor shape error");
        return -1;
    }
    float* f;
    ORT_ABORT_ON_ERROR(OrtGetTensorMutableData(tensor, (void**)&f));
    png_bytep model_output_bytes;
    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.format = PNG_FORMAT_BGR;
    image.height = dims[2];
    image.width = dims[3];
    chw_to_hwc(f, image.height, image.width, &model_output_bytes);
    int ret = 0;
    if (png_image_write_to_file(&image, output_file, 0 /*convert_to_8bit*/, model_output_bytes, 0 /*row_stride*/,
                                NULL /*colormap*/) == 0) {
        printf("write to '%s' failed:%s\n", output_file, image.message);
        ret = -1;
    }
    free(model_output_bytes);
    return ret;
}

static void usage() { printf("usage: <model_path> <input_file> <output_file> \n"); }

int run_inference(OrtSession* session, ORTCHAR_T* input_file, ORTCHAR_T* output_file) {
    size_t input_height;
    size_t input_width;
    float* model_input;
    size_t model_input_ele_count;

    char* input_file_p = input_file;
    char* output_file_p = output_file;
    if (read_png_file(input_file_p, &input_height, &input_width, &model_input, &model_input_ele_count) != 0) {
        return -1;
    }
    if (input_height != 720 || input_width != 720) {
        printf("please resize to image to 720x720\n");
        free(model_input);
        return -1;
    }
    OrtAllocatorInfo* allocator_info;
    ORT_ABORT_ON_ERROR(OrtCreateCpuAllocatorInfo(OrtArenaAllocator, OrtMemTypeDefault, &allocator_info));
    const int64_t input_shape[] = {1, 3, 720, 720};
    const size_t input_shape_len = sizeof(input_shape) / sizeof(input_shape[0]);
    const size_t model_input_len = model_input_ele_count * sizeof(float);

    OrtValue* input_tensor = NULL;
    ORT_ABORT_ON_ERROR(OrtCreateTensorWithDataAsOrtValue(allocator_info, model_input, model_input_len, input_shape,
                                                         input_shape_len, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
                                                         &input_tensor));
    assert(input_tensor != NULL);
    int is_tensor;
    ORT_ABORT_ON_ERROR(OrtIsTensor(input_tensor, &is_tensor));
    assert(is_tensor);
    OrtReleaseAllocatorInfo(allocator_info);
    const char* input_names[] = {"inputImage"};
    const char* output_names[] = {"outputImage"};
    OrtValue* output_tensor = NULL;
    ORT_ABORT_ON_ERROR(
            OrtRun(session, NULL, input_names, (const OrtValue* const*)&input_tensor, 1, output_names, 1, &output_tensor));
    assert(output_tensor != NULL);
    ORT_ABORT_ON_ERROR(OrtIsTensor(output_tensor, &is_tensor));
    assert(is_tensor);
    int ret = 0;
    if (write_tensor_to_png_file(output_tensor, output_file_p) != 0) {
        ret = -1;
    }
    OrtReleaseValue(output_tensor);
    OrtReleaseValue(input_tensor);
    free(model_input);

    return ret;
}

void verify_input_output_count(OrtSession* session) {
    size_t count;
    ORT_ABORT_ON_ERROR(OrtSessionGetInputCount(session, &count));
    assert(count == 1);
    ORT_ABORT_ON_ERROR(OrtSessionGetOutputCount(session, &count));
    assert(count == 1);
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        usage();
        return -1;
    }

    ORTCHAR_T* model_path = argv[1];
    ORTCHAR_T* input_file = argv[2];
    ORTCHAR_T* output_file = argv[3];
    OrtEnv* env;
    ORT_ABORT_ON_ERROR(OrtCreateEnv(ORT_LOGGING_LEVEL_WARNING, "test", &env));
    OrtSessionOptions* session_options;
    ORT_ABORT_ON_ERROR(OrtCreateSessionOptions(&session_options));

    OrtSession* session;
    ORT_ABORT_ON_ERROR(OrtCreateSession(env, model_path, session_options, &session));
    verify_input_output_count(session);
    int ret = run_inference(session, input_file, output_file);
    OrtReleaseSessionOptions(session_options);
    OrtReleaseSession(session);
    OrtReleaseEnv(env);
    if (ret != 0) {
        fprintf(stderr, "fail\n");
    }

    return ret;
}
