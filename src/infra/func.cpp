#include "infra.h"

namespace infra {
    std::string getFunctionFile(message::FunctionCall) {
        std::string filePath = "/usr/local/code/faasm/func/program.wasm";

        return filePath;
    }
}

