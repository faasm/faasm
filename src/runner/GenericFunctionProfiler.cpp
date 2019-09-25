#include "Profiler.h"

#include <boost/filesystem.hpp>

#define BINARY_DIR "/usr/local/code/faasm/cmake-build-debug/bin/"

namespace runner {
    GenericFunctionProfiler::GenericFunctionProfiler(std::string userIn, std::string funcName):
        Profiler(userIn, funcName, ""){

    }

    void GenericFunctionProfiler::runNative() {
        std::string execPath = std::string(BINARY_DIR) + funcName;

        // Check path exists
        if(!boost::filesystem::exists(execPath)) {
            throw std::runtime_error("Could not find binary at " + execPath);
        }

        system(execPath.c_str());
    }
}
