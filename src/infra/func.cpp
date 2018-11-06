#include "infra.h"

#include <util/util.h>
#include <boost/filesystem.hpp>

namespace infra {
    const static std::string funcFile = "function.wasm";
    const static std::string objFile = "function.o";

    boost::filesystem::path getFuncRootPath() {
        std::string funcRoot = util::getEnvVar("FUNC_ROOT", "/usr/local/code/faasm");

        boost::filesystem::path path(funcRoot);
        return path;
    }

    boost::filesystem::path getFunctionDir(const message::FunctionCall &call, bool create = true) {
        auto path = getFuncRootPath();

        path.append("wasm");
        path.append(call.user());
        path.append(call.function());

        // Create directory if doesn't exist
        if (create) {
            boost::filesystem::create_directories(path);
        }

        return path;
    }

    bool isValidFunction(const message::FunctionCall &call) {
        auto path = getFunctionDir(call, false);

        // Check object file exists
        path.append(objFile);
        bool isValid = boost::filesystem::exists(path);

        return isValid;
    }

    std::string getFunctionFile(const message::FunctionCall &call) {
        auto path = getFunctionDir(call);
        path.append(funcFile);

        return path.string();
    }

    std::string getFunctionObjectFile(const message::FunctionCall &call) {
        auto path = getFunctionDir(call);
        path.append(objFile);

        return path.string();
    }

    std::vector<uint8_t> getFunctionObjectBytes(const message::FunctionCall &call) {
        std::string objectFilePath = getFunctionObjectFile(call);
        std::vector<uint8_t> bytes = util::readFileToBytes(objectFilePath);

        return bytes;
    }

    std::vector<uint8_t> callToBytes(const message::FunctionCall &call) {
        size_t byteSize = call.ByteSizeLong();
        uint8_t buffer[byteSize];
        call.SerializeToArray(buffer, (int) byteSize);

        std::vector<uint8_t> inputData(buffer, buffer + byteSize);

        return inputData;
    }
}
