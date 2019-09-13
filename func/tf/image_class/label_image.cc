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

#include "bitmap_helpers.h"
#include "get_top_n.h"

#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/tools/evaluation/utils.h"

#define LOG(x) std::cerr

namespace tflite {
    namespace label_image {

        double get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

        using TfLiteDelegatePtr = tflite::Interpreter::TfLiteDelegatePtr;
        using TfLiteDelegatePtrMap = std::map<std::string, TfLiteDelegatePtr>;

        TfLiteDelegatePtrMap GetDelegates(Settings *s) {
            TfLiteDelegatePtrMap delegates;
            return delegates;
        }

// Takes a file name, and loads a list of labels from it, one per line, and
// returns a vector of the strings. It pads with empty strings so the length
// of the result is a multiple of 16, because our model expects that.
        TfLiteStatus ReadLabelsFile(const string &file_name,
                                    std::vector<string> *result,
                                    size_t *found_label_count) {
            std::ifstream file(file_name);
            if (!file) {
                LOG(FATAL) << "Labels file " << file_name << " not found\n";
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

        void PrintProfilingInfo(const profiling::ProfileEvent *e, uint32_t op_index,
                                TfLiteRegistration registration) {
            // output something like
            // time (ms) , Node xxx, OpCode xxx, symblic name
            //      5.352, Node   5, OpCode   4, DEPTHWISE_CONV_2D

            LOG(INFO) << std::fixed << std::setw(10) << std::setprecision(3)
                      << (e->end_timestamp_us - e->begin_timestamp_us) / 1000.0
                      << ", Node " << std::setw(3) << std::setprecision(3) << op_index
                      << ", OpCode " << std::setw(3) << std::setprecision(3)
                      << registration.builtin_code << ", "
                      << EnumNameBuiltinOperator(
                              static_cast<BuiltinOperator>(registration.builtin_code))
                      << "\n";
        }

        void RunInference(Settings *s) {
            if (!s->model_name.c_str()) {
                LOG(ERROR) << "no model file name\n";
                exit(-1);
            }

            std::unique_ptr<tflite::FlatBufferModel> model;
            std::unique_ptr<tflite::Interpreter> interpreter;
            model = tflite::FlatBufferModel::BuildFromFile(s->model_name.c_str());
            if (!model) {
                LOG(FATAL) << "\nFailed to mmap model " << s->model_name << "\n";
                exit(-1);
            }
            s->model = model.get();
            LOG(INFO) << "Loaded model " << s->model_name << "\n";
            model->error_reporter();
            LOG(INFO) << "resolved reporter\n";

            tflite::ops::builtin::BuiltinOpResolver resolver;

            tflite::InterpreterBuilder(*model, resolver)(&interpreter);
            if (!interpreter) {
                LOG(FATAL) << "Failed to construct interpreter\n";
                exit(-1);
            }

            interpreter->UseNNAPI(s->old_accel);
            interpreter->SetAllowFp16PrecisionForFp32(s->allow_fp16);

            if (s->verbose) {
                LOG(INFO) << "tensors size: " << interpreter->tensors_size() << "\n";
                LOG(INFO) << "nodes size: " << interpreter->nodes_size() << "\n";
                LOG(INFO) << "inputs: " << interpreter->inputs().size() << "\n";
                LOG(INFO) << "input(0) name: " << interpreter->GetInputName(0) << "\n";

                int t_size = interpreter->tensors_size();
                for (int i = 0; i < t_size; i++) {
                    if (interpreter->tensor(i)->name)
                        LOG(INFO) << i << ": " << interpreter->tensor(i)->name << ", "
                                  << interpreter->tensor(i)->bytes << ", "
                                  << interpreter->tensor(i)->type << ", "
                                  << interpreter->tensor(i)->params.scale << ", "
                                  << interpreter->tensor(i)->params.zero_point << "\n";
                }
            }

            if (s->number_of_threads != -1) {
                interpreter->SetNumThreads(s->number_of_threads);
            }

            int image_width = 224;
            int image_height = 224;
            int image_channels = 3;
            std::vector<uint8_t> in = read_bmp(s->input_bmp_name, &image_width,
                                               &image_height, &image_channels, s);

            int input = interpreter->inputs()[0];
            if (s->verbose) LOG(INFO) << "input: " << input << "\n";

            const std::vector<int> inputs = interpreter->inputs();
            const std::vector<int> outputs = interpreter->outputs();

            if (s->verbose) {
                LOG(INFO) << "number of inputs: " << inputs.size() << "\n";
                LOG(INFO) << "number of outputs: " << outputs.size() << "\n";
            }

            auto delegates_ = GetDelegates(s);
            for (const auto &delegate : delegates_) {
                if (interpreter->ModifyGraphWithDelegate(delegate.second.get()) !=
                    kTfLiteOk) {
                    LOG(FATAL) << "Failed to apply " << delegate.first << " delegate.";
                } else {
                    LOG(INFO) << "Applied " << delegate.first << " delegate.";
                }
            }

            if (interpreter->AllocateTensors() != kTfLiteOk) {
                LOG(FATAL) << "Failed to allocate tensors!";
            }

            if (s->verbose) PrintInterpreterState(interpreter.get());

            // get input dimension from the input tensor metadata
            // assuming one input only
            TfLiteIntArray *dims = interpreter->tensor(input)->dims;
            int wanted_height = dims->data[1];
            int wanted_width = dims->data[2];
            int wanted_channels = dims->data[3];

            switch (interpreter->tensor(input)->type) {
                case kTfLiteFloat32:
                    s->input_floating = true;
                    resize<float>(interpreter->typed_tensor<float>(input), in.data(),
                                  image_height, image_width, image_channels, wanted_height,
                                  wanted_width, wanted_channels, s);
                    break;
                case kTfLiteUInt8:
                    resize<uint8_t>(interpreter->typed_tensor<uint8_t>(input), in.data(),
                                    image_height, image_width, image_channels, wanted_height,
                                    wanted_width, wanted_channels, s);
                    break;
                default:
                    LOG(FATAL) << "cannot handle input type "
                               << interpreter->tensor(input)->type << " yet";
                    exit(-1);
            }

            auto profiler =
                    absl::make_unique<profiling::Profiler>(s->max_profiling_buffer_entries);
            interpreter->SetProfiler(profiler.get());

            if (s->profiling) profiler->StartProfiling();
            if (s->loop_count > 1)
                for (int i = 0; i < s->number_of_warmup_runs; i++) {
                    if (interpreter->Invoke() != kTfLiteOk) {
                        LOG(FATAL) << "Failed to invoke tflite!\n";
                    }
                }

            struct timeval start_time, stop_time;
            gettimeofday(&start_time, nullptr);
            for (int i = 0; i < s->loop_count; i++) {
                if (interpreter->Invoke() != kTfLiteOk) {
                    LOG(FATAL) << "Failed to invoke tflite!\n";
                }
            }
            gettimeofday(&stop_time, nullptr);
            LOG(INFO) << "invoked \n";
            LOG(INFO) << "average time: "
                      << (get_us(stop_time) - get_us(start_time)) / (s->loop_count * 1000)
                      << " ms \n";

            if (s->profiling) {
                profiler->StopProfiling();
                auto profile_events = profiler->GetProfileEvents();
                for (int i = 0; i < profile_events.size(); i++) {
                    auto op_index = profile_events[i]->event_metadata;
                    const auto node_and_registration =
                            interpreter->node_and_registration(op_index);
                    const TfLiteRegistration registration = node_and_registration->second;
                    PrintProfilingInfo(profile_events[i], op_index, registration);
                }
            }

            const float threshold = 0.001f;

            std::vector<std::pair<float, int>> top_results;

            int output = interpreter->outputs()[0];
            TfLiteIntArray *output_dims = interpreter->tensor(output)->dims;
            // assume output dims to be something like (1, 1, ... ,size)
            auto output_size = output_dims->data[output_dims->size - 1];
            switch (interpreter->tensor(output)->type) {
                case kTfLiteFloat32:
                    get_top_n<float>(interpreter->typed_output_tensor<float>(0), output_size,
                                     s->number_of_results, threshold, &top_results, true);
                    break;
                case kTfLiteUInt8:
                    get_top_n<uint8_t>(interpreter->typed_output_tensor<uint8_t>(0),
                                       output_size, s->number_of_results, threshold,
                                       &top_results, false);
                    break;
                default:
                    LOG(FATAL) << "cannot handle output type "
                               << interpreter->tensor(input)->type << " yet";
                    exit(-1);
            }

            std::vector<string> labels;
            size_t label_count;

            if (ReadLabelsFile(s->labels_file_name, &labels, &label_count) != kTfLiteOk)
                exit(-1);

            for (const auto &result : top_results) {
                const float confidence = result.first;
                const int index = result.second;
                LOG(INFO) << confidence << ": " << index << " " << labels[index] << "\n";
            }
        }

        int Main(int argc, char **argv) {
# if WASM_BUILD == 1
            std::string dataDir = "/runtime_data/";
#else
            std::string dataDir = "/usr/local/code/faasm/func/tf/data/";
#endif
            std::string imagePath = dataDir + "grace_hopper.bmp";
            std::string labelsPath = dataDir + "labels.txt";
            std::string modelsPath = dataDir + "mobilenet_v1_1.0_224.tflite";

            Settings s;
            s.accel = false;
            s.loop_count = 10;
            s.input_bmp_name = imagePath;
            s.labels_file_name = labelsPath;
            s.model_name = modelsPath;
            s.number_of_threads = 1;

            RunInference(&s);
            return 0;
        }

    }
}

int main(int argc, char **argv) {
    return tflite::label_image::Main(argc, argv);
}
