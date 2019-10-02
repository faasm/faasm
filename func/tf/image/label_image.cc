#include "label_image.h"

#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "absl/memory/memory.h"

#include "get_top_n.h"

#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/tools/evaluation/utils.h"

#include <faasm/time.h>

namespace tflite {
    namespace label_image {
        using TfLiteDelegatePtr = tflite::Interpreter::TfLiteDelegatePtr;
        using TfLiteDelegatePtrMap = std::map<std::string, TfLiteDelegatePtr>;

        // Takes a file name, and loads a list of labels from it, one per line, and
        // returns a vector of the strings. It pads with empty strings so the length
        // of the result is a multiple of 16, because our model expects that.
        TfLiteStatus ReadLabelsFile(const string &file_name,
                                    std::vector<string> *result,
                                    size_t *found_label_count) {
            FAASM_PROF_START(labelsFile)

            std::ifstream file(file_name);
            if (!file) {
                printf("Labels file %s not found\n", file_name.c_str());
                return kTfLiteError;
            }

            result->clear();
            string line;
            while (std::getline(file, line)) {
                result->push_back(line);
            }

            *found_label_count = result->size();
            const int padding = 16;
            while (result->size() % padding) {
                result->emplace_back();
            }

            FAASM_PROF_END(labelsFile)
            return kTfLiteOk;
        }
    }
}

int main(int argc, char **argv) {
# if __wasm__ == 1
    std::string dataDir = "faasm://tfdata/";
#else
    std::string dataDir = "/usr/local/code/faasm/func/tf/data/";
#endif
    std::string imagePath = dataDir + "grace_hopper.bmp";
    std::string labelsPath = dataDir + "labels.txt";
    std::string modelsPath = dataDir + "mobilenet_v1_1.0_224.tflite";

    tflite::label_image::Settings s;
    s.accel = false;
    s.loop_count = 1;
    s.input_bmp_name = imagePath;
    s.labels_file_name = labelsPath;
    s.model_name = modelsPath;
    s.number_of_threads = 1;
    s.number_of_warmup_runs = 0;
    s.allow_fp16 = true;

    FAASM_PROF_START(buildModel)
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    model = tflite::FlatBufferModel::BuildFromFile(s.model_name.c_str());
    FAASM_PROF_END(buildModel)

    if (!model) {
        printf("\nFailed to mmap model %s\n", s.model_name.c_str());
        exit(-1);
    }

    FAASM_PROF_START(getModel)
    s.model = model.get();
    printf("Loaded model %s\n", s.model_name.c_str());
    model->error_reporter();
    FAASM_PROF_END(getModel)

    FAASM_PROF_START(buildInterpreter)
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    if (!interpreter) {
        printf("Failed to construct interpreter\n");
        exit(-1);
    }
    FAASM_PROF_END(buildInterpreter)

    interpreter->UseNNAPI(false);
    interpreter->SetAllowFp16PrecisionForFp32(true);
    interpreter->SetNumThreads(1);

    FAASM_PROF_START(readImage)
    int image_width = 224;
    int image_height = 224;
    int image_channels = 3;
    printf("Reading in image %s\n", s.input_bmp_name.c_str());
    std::vector<uint8_t> in = tflite::label_image::read_bmp(
            s.input_bmp_name,
            &image_width,
            &image_height,
            &image_channels
    );
    printf("Finished reading in image %s\n", s.input_bmp_name.c_str());
    printf("Got w, h, c: %i, %i, %i\n", image_width, image_height, image_channels);
    FAASM_PROF_END(readImage)

    int input = interpreter->inputs()[0];
    const std::vector<int> inputs = interpreter->inputs();

    printf("Allocating tensors\n");
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        printf("Failed to allocate tensors!\n");
    }
    printf("Finished allocating tensors\n");

    // get input dimension from the input tensor metadata
    // assuming one input only
    TfLiteIntArray *dims = interpreter->tensor(input)->dims;
    int wanted_height = dims->data[1];
    int wanted_width = dims->data[2];
    int wanted_channels = dims->data[3];

    FAASM_PROF_START(resizeImage)

    s.input_floating = true;
    tflite::label_image::resize(
            interpreter->typed_tensor<float>(input),
            in.data(),
            image_height,
            image_width,
            image_channels,
            wanted_height,
            wanted_width,
            wanted_channels,
            &s
    );

    FAASM_PROF_END(resizeImage)

    FAASM_PROF_START(warmUpRuns)
    if (s.loop_count > 1) {
        for (int i = 0; i < s.number_of_warmup_runs; i++) {
            if (interpreter->Invoke() != kTfLiteOk) {
                printf("Failed to invoke tflite!\n");
            }
        }
    }
    FAASM_PROF_END(warmUpRuns)

    FAASM_PROF_START(interpreterLoop)
    printf("Invoking interpreter in a loop\n");
    for (int i = 0; i < s.loop_count; i++) {
        printf("Interpreter invoke %i\n", i);

        if (interpreter->Invoke() != kTfLiteOk) {
            printf("Failed to invoke tflite!\n");
        }
    }
    FAASM_PROF_END(interpreterLoop)

    printf("Finished invoking\n");

    std::vector<int> outputs = interpreter->outputs();
    unsigned long outputsSize = outputs.size();
    if (outputsSize == 0) {
        printf("Empty result from interpreter\n");
        exit(1);
    }

    int output = outputs[0];
    TfLiteIntArray *output_dims = interpreter->tensor(output)->dims;

    // assume output dims to be something like (1, 1, ... ,size)
    FAASM_PROF_START(getTopResults)

    const float threshold = 0.001f;
    std::vector<std::pair<float, int>> top_results;
    auto output_size = output_dims->data[output_dims->size - 1];

    tflite::label_image::get_top_n<float>(
            interpreter->typed_output_tensor<float>(0),
            output_size,
            s.number_of_results,
            threshold,
            &top_results,
            true
    );

    FAASM_PROF_END(getTopResults)

    if (top_results.empty()) {
        printf("No top results found\n");
        exit(1);
    } else {
        printf("Found %li top results\n", top_results.size());
    }

    std::vector<std::string> labels;
    size_t label_count;

    FAASM_PROF_START(readLabels)
    if (tflite::label_image::ReadLabelsFile(
            s.labels_file_name,
            &labels,
            &label_count
    ) != kTfLiteOk) {
        printf("Failed reading labels file: %s\n", s.labels_file_name.c_str());
        exit(-1);
    }
    FAASM_PROF_END(readLabels)

    for (const auto &result : top_results) {
        const float confidence = result.first;
        const int index = result.second;
        printf("%f: %i %s\n", confidence, index, labels[index].c_str());
    }

    return 0;
}
