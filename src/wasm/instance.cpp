#include <util/config.h>

#include "LocalFunctionLoader.h"
#include "S3FunctionLoader.h"

namespace wasm {
    FunctionLoader &getFunctionLoader() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.systemMode == "redis") {
            static LocalFunctionLoader fl;
            return fl;
        } else if (conf.systemMode == "aws") {
            static S3FunctionLoader fl;
            return fl;
        } else {
            throw std::runtime_error("Invalid system mode");
        }
    }
}
