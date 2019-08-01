#pragma once

#include <util/config.h>
#include "FunctionLoader.h"

namespace wasm {
    class LocalFunctionLoader : public FunctionLoader {
    public:
        std::vector<uint8_t> loadFunctionBytes(const message::Message &msg);

        std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg);

        std::vector<uint8_t> loadFileBytes(const std::string &path);

        std::vector<uint8_t> loadPythonFunction(const message::Message &msg);

        void uploadFunction(message::Message &msg);

        void uploadPythonFunction(message::Message &msg);

        void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes);

        void uploadObjectBytes(const std::string &path, const std::vector<uint8_t> &objBytes);
    };
};