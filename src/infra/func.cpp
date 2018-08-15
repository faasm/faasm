#include "infra.h"

#include <util/util.h>
#include <boost/filesystem.hpp>

namespace infra {
    std::string getFunctionDir(const message::FunctionCall &call) {
        std::string projRoot = util::getEnvVar("FUNC_ROOT", "/usr/local/code/faasm");
        std::string dirStr = projRoot + "/wasm/" + call.user() + "/" + call.function();

        // Create directory if doesn't exist
        boost::filesystem::create_directories(dirStr);

        return dirStr;
    }

    std::string getFunctionFile(const message::FunctionCall &call) {
        std::string dirPath = getFunctionDir(call);
        std::string filePath = dirPath + "/function.wasm";

        return filePath;
    }
}
