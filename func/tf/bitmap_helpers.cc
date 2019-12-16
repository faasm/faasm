#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "image.h"

#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"

namespace tflite {
    namespace label_image {

        std::vector<uint8_t> decode_bmp(const uint8_t *input, int row_size, int width,
                                        int height, int channels, bool top_down) {
            std::vector<uint8_t> output(height * width * channels);
            for (int i = 0; i < height; i++) {
                int src_pos;
                int dst_pos;

                for (int j = 0; j < width; j++) {
                    if (!top_down) {
                        src_pos = ((height - 1 - i) * row_size) + j * channels;
                    } else {
                        src_pos = i * row_size + j * channels;
                    }

                    dst_pos = (i * width + j) * channels;

                    if (channels == 1) {
                        output[dst_pos] = input[src_pos];
                    } else if (channels == 3) {
                        // BGR -> RGB
                        output[dst_pos] = input[src_pos + 2];
                        output[dst_pos + 1] = input[src_pos + 1];
                        output[dst_pos + 2] = input[src_pos];
                    } else if (channels == 4) {
                        // BGRA -> RGBA
                        output[dst_pos] = input[src_pos + 2];
                        output[dst_pos + 1] = input[src_pos + 1];
                        output[dst_pos + 2] = input[src_pos];
                        output[dst_pos + 3] = input[src_pos + 3];
                    } else {
                        printf("Unexpected number of channels: %i\n", channels);
                    }
                }
            }
            return output;
        }

        std::vector<uint8_t> read_bmp(const std::string &input_bmp_name, int *width,
                                      int *height, int *channels) {

            FILE *f = fopen(input_bmp_name.c_str(), "rb");
            unsigned char info[54];

            // read the 54-byte header
            fread(info, sizeof(unsigned char), 54, f);

            // extract image height and width from header
            int imgWidth = *(int *) &info[18];
            int imgHeight = *(int *) &info[22];

            // Allocate 3 bytes per pixel
            int size = 3 * imgWidth * imgHeight;

            // Read in all the data including the header
            int totalSize = size + 54;
            auto data = new unsigned char[totalSize];

            // Close the file
            fclose(f);

            // Read the rest of the data at once
            FILE *fNew = fopen(input_bmp_name.c_str(), "rb");
            fread(data, sizeof(unsigned char), totalSize, fNew);
            fclose(fNew);

            std::vector<uint8_t> img_bytes(data, data + totalSize);
            const int32_t header_size = *(reinterpret_cast<const int32_t *>(img_bytes.data() + 10));

            *width = *(reinterpret_cast<const int32_t *>(img_bytes.data() + 18));
            *height = *(reinterpret_cast<const int32_t *>(img_bytes.data() + 22));
            const int32_t bpp =
                    *(reinterpret_cast<const int32_t *>(img_bytes.data() + 28));
            *channels = bpp / 8;

            printf("Image read in:\n");
            printf("width, height, channels: %i, %i, %i \n", *width, *height, *channels);

            // there may be padding bytes when the width is not a multiple of 4 bytes
            // 8 * channels == bits per pixel
            const int row_size = (8 * *channels * *width + 31) / 32 * 4;

            // if height is negative, data layout is top down
            // otherwise, it's bottom up
            bool top_down = (*height < 0);

            // Decode image, allocating tensor once the image size is known
            const uint8_t *bmp_pixels = &img_bytes[header_size];
            const std::vector<uint8_t> result = decode_bmp(bmp_pixels, row_size, *width, abs(*height), *channels,
                                                           top_down);

            // delete[] data;
            return result;
        }

        void resize(float *out, unsigned char *in, int image_height, int image_width,
                    int image_channels, int wanted_height, int wanted_width,
                    int wanted_channels, Settings *s) {
            int number_of_pixels = image_height * image_width * image_channels;
            std::unique_ptr<Interpreter> interpreter(new Interpreter);

            int base_index = 0;

            // two inputs: input and new_sizes
            interpreter->AddTensors(2, &base_index);
            // one output
            interpreter->AddTensors(1, &base_index);

            // set input and output tensors
            interpreter->SetInputs({0, 1});
            interpreter->SetOutputs({2});

            // set parameters of tensors
            TfLiteQuantizationParams quant;
            interpreter->SetTensorParametersReadWrite(
                    0, kTfLiteFloat32, "input",
                    {1, image_height, image_width, image_channels}, quant);

            interpreter->SetTensorParametersReadWrite(1, kTfLiteInt32, "new_size", {2},
                                                      quant);

            interpreter->SetTensorParametersReadWrite(
                    2, kTfLiteFloat32, "output",
                    {1, wanted_height, wanted_width, wanted_channels}, quant);

            ops::builtin::BuiltinOpResolver resolver;
            const TfLiteRegistration *resize_op =
                    resolver.FindOp(BuiltinOperator_RESIZE_BILINEAR, 1);

            auto *params = reinterpret_cast<TfLiteResizeBilinearParams *>(
                    malloc(sizeof(TfLiteResizeBilinearParams)));

            params->align_corners = false;
            interpreter->AddNodeWithParameters({0, 1}, {2}, nullptr, 0, params, resize_op,
                                               nullptr);

            interpreter->AllocateTensors();

            // fill input image
            // in[] are integers, cannot do memcpy() directly
            float *input = interpreter->typed_tensor<float>(0);
            for (int i = 0; i < number_of_pixels; i++) {
                input[i] = (float) in[i];
            }

            // fill new_sizes
            interpreter->typed_tensor<int>(1)[0] = wanted_height;
            interpreter->typed_tensor<int>(1)[1] = wanted_width;

            interpreter->Invoke();

            float *output = interpreter->typed_tensor<float>(2);
            int output_number_of_pixels = wanted_height * wanted_width * wanted_channels;

            for (int i = 0; i < output_number_of_pixels; i++) {
                out[i] = (output[i] - s->input_mean) / s->input_std;
            }
        }

    }  // namespace label_image
}  // namespace tflite
