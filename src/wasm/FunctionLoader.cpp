#include "FunctionLoader.h"

#include <WAVM/IR/Module.h>
#include <WAVM/Inline/BasicTypes.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/Inline/Errors.h>
#include <WAVM/Inline/Serialization.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <util/config.h>

using namespace WAVM;

namespace wasm {

    bool FunctionLoader::isWasm(const std::vector<uint8_t> &bytes) {
        static const U8 wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if (bytes.size() >= 4 && !memcmp(bytes.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            return false;
        }
    }

    void FunctionLoader::compileToObjectFile(message::Message &msg) {
        std::vector<uint8_t> bytes = this->loadFunctionBytes(msg);

        std::vector<uint8_t> objBytes = this->doCompile(bytes);

        this->uploadObjectBytes(msg, objBytes);
    }

    void FunctionLoader::compileToObjectFile(const std::string &inputPath, const std::string &outputPath) {
        std::vector<uint8_t> bytes = this->loadFileBytes(inputPath);

        std::vector<uint8_t> objBytes = this->doCompile(bytes);

        this->uploadObjectBytes(outputPath, objBytes);
    }

    std::vector<uint8_t> FunctionLoader::doCompile(std::vector<uint8_t> &bytes) {
        IR::Module moduleIR;
        if (this->isWasm(bytes)) {
            // Handle WASM
            bool success = WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR);
            if (!success) {
                throw std::runtime_error("Failed to parse wasm binary");
            }
        } else {
            std::vector<WAST::Error> parseErrors;
            bool success = WAST::parseModule((const char *) bytes.data(), bytes.size(), moduleIR, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);

            if (!success) {
                throw std::runtime_error("Failed to parse wast file");
            }
        }

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);
        return objBytes;
    }


}
