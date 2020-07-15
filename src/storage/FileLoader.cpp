#include "FileLoader.h"

#include <WAVM/IR/Module.h>
#include <WAVM/Inline/BasicTypes.h>
#include <WAVM/Inline/Serialization.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <util/config.h>
#include <util/func.h>
#include <util/files.h>
#include <util/logging.h>

#include <boost/filesystem.hpp>


using namespace WAVM;

namespace storage {

    bool FileLoader::isWasm(const std::vector<uint8_t> &bytes) {
        static const U8 wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if (bytes.size() >= 4 && !memcmp(bytes.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            return false;
        }
    }

    void FileLoader::codegenForFunction(message::Message &msg) {
        std::vector<uint8_t> bytes = loadFunctionWasm(msg);

        if(bytes.empty()) {
            const std::string funcStr = util::funcToString(msg, false);
            throw std::runtime_error("Loaded empty bytes for " + funcStr);
        }
        
        std::vector<uint8_t> objBytes = doCodegen(bytes);

        uploadFunctionObjectFile(msg, objBytes);
    }

    void FileLoader::codegenForSharedObject(const std::string &inputPath) {
        // Generate the machine code
        std::vector<uint8_t> bytes = loadSharedObjectWasm(inputPath);
        std::vector<uint8_t> objBytes = doCodegen(bytes);

        // Do the upload
        uploadSharedObjectObjectFile(inputPath, objBytes);
    }

    std::vector<uint8_t> FileLoader::doCodegen(std::vector<uint8_t> &bytes) {
        IR::Module moduleIR;

        // Explicitly allow simd support
        moduleIR.featureSpec.simd = true;
        moduleIR.featureSpec.simdBitMask = true;
        moduleIR.featureSpec.atomics = true;

        if (this->isWasm(bytes)) {
            // Handle WASM
            WASM::LoadError loadError;
            bool success = WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR, &loadError);
            if (!success) {
                throw std::runtime_error("Failed to parse wasm binary: " + loadError.message);
            }
        } else {
            std::vector<WAST::Error> parseErrors;
            bool success = WAST::parseModule((const char *) bytes.data(), bytes.size(), moduleIR, parseErrors);
            WAST::reportParseErrors("wast_file", (const char*)bytes.data(), parseErrors);

            if (!success) {
                throw std::runtime_error("Failed to parse wast file");
            }
        }

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);
        return objBytes;
    }

    void checkFileExists(const std::string &path) {
        if (!boost::filesystem::exists(path)) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("File {} does not exist", path);
            throw std::runtime_error("Expected file does not exist");
        }
    }

    std::vector<uint8_t> loadFileBytes(const std::string &path) {
        checkFileExists(path);
        return util::readFileToBytes(path);
    }
}
