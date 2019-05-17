#include "Profiler.h"

#include <stdlib.h>

#define POLYBENCH_USER "polybench"

#define BINARY_DIR "/usr/local/code/faasm/cmake-build-release/bin/"

namespace runner {
    PolybenchProfiler::PolybenchProfiler(std::string funcName) : Profiler(POLYBENCH_USER, funcName, "") {

    }

    void PolybenchProfiler::runNative() {
        // TODO - avoid use of system here
        std::string execPath = std::string(BINARY_DIR) + funcName;

        system(execPath.c_str());
    }
}