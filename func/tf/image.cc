#include "image.h"

#include <faasm/time.h>
#include <faasm/input.h>
#include <faasm/faasm.h>

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

#ifndef __wasm__
#include <emulator/emulator.h>
#endif

#include "absl/memory/memory.h"

#include "get_top_n.h"

#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/tools/evaluation/utils.h"

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

            return kTfLiteOk;
        }
    }
}

FAASM_MAIN_FUNC() {

#ifdef __wasm__
    std::string dataDir = "faasm://tfdata/";
#else
    std::string dataDir = "/usr/local/code/faasm/func/tf/data/";
    setEmulatorUser("tf");
#endif

    int loopCount = 1;
    int warmupLoops = 0;
    int nResults = 5;

    std::string imagePath = dataDir + "grace_hopper.bmp";
    std::string labelsPath = dataDir + "labels.txt";

    std::string modelKey = "mobilenet_v1";

    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;

    const size_t modelSize = 16900760;

    FAASM_PROF_START(modelRead)
#ifdef __wasm__
    // With wasm we can read directly to a pointer from shared state
    uint8_t *modelBytes = faasmReadStatePtr(modelKey.c_str(), modelSize);
#else
    auto modelBytes = new uint8_t[modelSize];
    faasmReadState(modelKey.c_str(), modelBytes, modelSize);
#endif
    FAASM_PROF_END(modelRead)

    FAASM_PROF_START(modelBuild)
    model = tflite::FlatBufferModel::BuildFromBuffer(reinterpret_cast<char *>(modelBytes), modelSize);

    if (!model) {
        printf("\nFailed to load model from key %s\n", modelKey.c_str());
        exit(-1);
    }

    printf("Loaded model %s\n", modelKey.c_str());
    model->error_reporter();

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    if (!interpreter) {
        printf("Failed to construct interpreter\n");
        exit(-1);
    }

    interpreter->UseNNAPI(false);
    interpreter->SetAllowFp16PrecisionForFp32(false);
    interpreter->SetNumThreads(1);
    FAASM_PROF_END(modelBuild)

    FAASM_PROF_START(imgRead)
    int image_width = 224;
    int image_height = 224;
    int image_channels = 3;
    printf("Reading in image %s\n", imagePath.c_str());
    std::vector<uint8_t> in = tflite::label_image::read_bmp(
            imagePath.c_str(),
            &image_width,
            &image_height,
            &image_channels
    );
    printf("Finished reading in image %s\n", imagePath.c_str());
    printf("Got w, h, c: %i, %i, %i\n", image_width, image_height, image_channels);
    FAASM_PROF_END(imgRead)

    FAASM_PROF_START(tensors)
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
    FAASM_PROF_END(tensors)

    FAASM_PROF_START(imgResize)
    tflite::label_image::resize(
            interpreter->typed_tensor<float>(input),
            in.data(),
            image_height,
            image_width,
            image_channels,
            wanted_height,
            wanted_width,
            wanted_channels
    );
    FAASM_PROF_END(imgResize)

    FAASM_PROF_START(interpreterLoops)
    if (loopCount > 1) {
        for (int i = 0; i < warmupLoops; i++) {
            if (interpreter->Invoke() != kTfLiteOk) {
                printf("Failed to invoke tflite!\n");
            }
        }
    }

    printf("Invoking interpreter in a loop\n");
    for (int i = 0; i < loopCount; i++) {
        printf("Interpreter invoke %i\n", i);

        if (interpreter->Invoke() != kTfLiteOk) {
            printf("Failed to invoke tflite!\n");
        }
    }

    FAASM_PROF_END(interpreterLoops)

    FAASM_PROF_START(outputPrep)
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

    const float threshold = 0.001f;
    std::vector<std::pair<float, int>> top_results;
    auto output_size = output_dims->data[output_dims->size - 1];

    tflite::label_image::get_top_n<float>(
            interpreter->typed_output_tensor<float>(0),
            output_size,
            nResults,
            threshold,
            &top_results,
            true
    );


    if (top_results.empty()) {
        printf("No top results found\n");
        exit(1);
    } else {
        printf("Found %li top results\n", top_results.size());
    }
    FAASM_PROF_END(outputPrep)

    FAASM_PROF_START(labelsRead)
    std::vector<std::string> labels;
    size_t label_count;

    if (tflite::label_image::ReadLabelsFile(
            labelsPath.c_str(),
            &labels,
            &label_count
    ) != kTfLiteOk) {
        printf("Failed reading labels file: %s\n", labelsPath.c_str());
        exit(-1);
    }

    std::string outputStr;
    for (const auto &result : top_results) {
        const float confidence = result.first;
        const int index = result.second;
        printf("%f: %i %s\n", confidence, index, labels[index].c_str());
        outputStr += std::to_string(confidence) + ": " + std::to_string(index) + " " + labels[index] + "\n";
    }

    faasm::setStringOutput(outputStr.c_str());
    FAASM_PROF_END(labelsRead)

#ifndef __wasm__
    delete[] modelBytes;
#endif

    return 0;
}
