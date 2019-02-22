#pragma once

#include <proto/faasm.pb.h>

#include <string>
#include <vector>

namespace wasm {
    class FunctionLoader {
    public:
        virtual std::vector<uint8_t> loadFunctionBytes(const message::Message &msg) = 0;

        virtual std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg) = 0;

        virtual void uploadFunction(message::Message &msg) = 0;

        virtual void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) = 0;

        void compileToObjectFile(message::Message &msg);
    };

    FunctionLoader &getFunctionLoader();
};