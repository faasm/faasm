#include "infra.h"

namespace infra {
    std::string getFunctionFile(message::FunctionCall &call) {
        std::string filePath = "/usr/local/code/faasm/wasm/" + call.user() + "/" + call.function() + "/function.wasm";
        return filePath;
    }
}

