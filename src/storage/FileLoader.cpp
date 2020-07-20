#include "FileLoader.h"

#include <util/config.h>
#include <util/func.h>
#include <util/files.h>
#include <util/logging.h>

#include <boost/filesystem.hpp>
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>


namespace storage {

    std::vector<uint8_t> FileLoader::doCodegen(std::vector<uint8_t> &bytes) {
        util::SystemConfig &conf = util::getSystemConfig();
        if(conf.wasmVm == "wamr") {
            wasm::WAMRWasmModule module;
            std::vector<uint8_t> machineCode = module.codegen(bytes);
            return machineCode;
        } else {
            wasm::WAVMWasmModule module;
            std::vector<uint8_t> machineCode = module.codegen(bytes);
            return machineCode;
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
