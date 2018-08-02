#include "infra.h"

namespace infra {
    std::string getFunctionFile(message::FunctionCall &call) {
        std::string filePath = FUNCTION_DIR + call.user() + "/" + call.function() + "/function.wasm";
        return filePath;
    }
}

