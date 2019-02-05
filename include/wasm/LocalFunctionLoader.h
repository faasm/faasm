#pragma once

#include "FunctionLoader.h"

namespace wasm {
    class LocalFunctionLoader : public FunctionLoader {
    public:
        std::vector<uint8_t> loadFunctionBytes(const message::Message &msg);

        std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg);

        void uploadFunction(message::Message &msg);

        void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes);
    };
};