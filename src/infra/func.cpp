#include "infra.h"

#include <util/util.h>

namespace infra {
    std::string getFunctionFile(const message::FunctionCall &call) {
        std::string projRoot = util::getEnvVar("FUNC_ROOT", "/usr/local/code/faasm");
        std::string filePath = projRoot + "/wasm/" + call.user() + "/" + call.function() + "/function.wasm";

        return filePath;
    }
}
