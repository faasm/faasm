#include "FunctionLoader.h"

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <util/config.h>

using namespace WAVM;

namespace wasm {

    bool FunctionLoader::isWasm(std::vector<uint8_t> &bytes) {
        static const U8 wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if(bytes.size() >= 4 && !memcmp(bytes.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            // Ensure null terminated
            bytes.push_back(0);
            return false;
        }
    }

    void FunctionLoader::compileToObjectFile(message::Message &msg) {
        // Parse the wasm file to work out imports, function signatures etc.
        IR::Module moduleIR;
        std::vector<uint8_t> bytes = this->loadFunctionBytes(msg);

        if(this->isWasm(bytes)) {
            // Handle WASM
            bool success = WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR);
            if(!success) {
                throw std::runtime_error("Failed to parse wasm binary");
            }
        } else {
            std::vector<WAST::Error> parseErrors;
            bool success = WAST::parseModule((const char *) bytes.data(), bytes.size(), moduleIR, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);

            if(!success) {
                throw std::runtime_error("Failed to parse wast file");
            }
        }

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);

        this->uploadObjectBytes(msg, objBytes);
    }
}
