#include "Profiler.h"

#include <stdio.h>
#include <stdlib.h>

#define POLYBENCH_USER "polybench"

#define BINARY_DIR "/usr/local/code/faasm/build/polybench_native/bin/"

namespace runner {
    PolybenchProfiler::PolybenchProfiler(std::string funcName) : Profiler(POLYBENCH_USER, funcName, "") {

    }

    void PolybenchProfiler::runNative() {
        std::string execPath = std::string(BINARY_DIR) + funcName;

        // TODO - is it possible to avoid system here?
        int error = system(execPath.c_str());
        if (error != 0)
            printf("Failed to execute %s", execPath.c_str());
    }
}
