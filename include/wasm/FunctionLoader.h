#pragma once

#include <proto/faasm.pb.h>

#include <string>
#include <vector>

#define SHARED_OBJ_EXT ".o"

namespace wasm {
    class FunctionLoader {
    public:
        virtual std::vector<uint8_t> loadFunctionBytes(const message::Message &msg) = 0;

        virtual std::vector<uint8_t> loadFunctionBytes(const std::string &path) = 0;

        virtual std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg) = 0;

        virtual std::vector<uint8_t> loadFunctionObjectBytes(const std::string &path) = 0;

        virtual void uploadFunction(message::Message &msg) = 0;

        virtual void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) = 0;

        virtual void uploadObjectBytes(const std::string &path, const std::vector<uint8_t> &objBytes) = 0;

        void compileToObjectFile(message::Message &msg);

        void compileToObjectFile(const std::string &inputPath, const std::string &outputPath);

        bool isWasm(const std::vector<uint8_t> &bytes);

        std::vector<uint8_t> doCompile(std::vector<uint8_t> &bytes);
    };

    FunctionLoader &getFunctionLoader();
};