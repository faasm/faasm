#pragma once

#include <proto/faasm.pb.h>

#include <WAVM/WASM/WASM.h>

#include <string>
#include <vector>

using namespace WAVM;

namespace wasm {
    class FunctionLoader {
    public:
        void loadFunctionBytes(const message::Message &msg, IR::Module& outModule);

        void uploadFunction(message::Message &msg);

    private:
        void compileToObjectFile(message::Message &msg);
    };
};