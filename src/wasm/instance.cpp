#include <util/config.h>

#include "LocalFunctionLoader.h"
#include "FileserverFunctionLoader.h"
#include "S3FunctionLoader.h"

namespace wasm {
    FunctionLoader &getFunctionLoader() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.functionStorage == "local") {
            static thread_local LocalFunctionLoader fl;
            return fl;
        } else if (conf.functionStorage == "s3") {
            static thread_local S3FunctionLoader fl;
            return fl;
        } else if (conf.functionStorage == "fileserver") {
            static thread_local FileserverFunctionLoader fl;
            return fl;
        } else {
            throw std::runtime_error("Invalid function storage mode");
        }
    }
}
