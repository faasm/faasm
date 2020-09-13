#include "FileLoader.h"

#if(FAASM_SGX == 1)
#else
#include <wamr/WAMRWasmModule.h>
#endif

#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/files.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>


namespace storage {

    std::vector<uint8_t> FileLoader::doCodegen(std::vector<uint8_t> &bytes) {
        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
        if (conf.wasmVm == "wamr") {
#if(FAASM_SGX || WAMR_EXECUTION_MODE_INTERP)
            throw std::runtime_error("WAMR codegen not supported in current wasm execution mode");
#else
            return wasm::wamrCodegen(bytes);
#endif
        } else {
            return wasm::wavmCodegen(bytes);
        }
    }

    void FileLoader::codegenForFunction(faabric::Message &msg) {
        std::vector<uint8_t> bytes = loadFunctionWasm(msg);

        if (bytes.empty()) {
            const std::string funcStr = faabric::util::funcToString(msg, false);
            throw std::runtime_error("Loaded empty bytes for " + funcStr);
        }

        std::vector<uint8_t> objBytes = doCodegen(bytes);

        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
        if (conf.wasmVm == "wamr") {
            uploadFunctionAotFile(msg, objBytes);
        } else {
            uploadFunctionObjectFile(msg, objBytes);
        }
    }

    void FileLoader::codegenForSharedObject(const std::string &inputPath) {
        // Generate the machine code
        std::vector<uint8_t> bytes = loadSharedObjectWasm(inputPath);
        std::vector<uint8_t> objBytes = doCodegen(bytes);

        // Do the upload
        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
        if (conf.wasmVm == "wamr") {
            uploadSharedObjectAotFile(inputPath, objBytes);
        } else {
            uploadSharedObjectObjectFile(inputPath, objBytes);
        }
    }

    void checkFileExists(const std::string &path) {
        if (!boost::filesystem::exists(path)) {
            const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
            logger->error("File {} does not exist", path);
            throw std::runtime_error("Expected file does not exist");
        }
    }

    std::vector<uint8_t> loadFileBytes(const std::string &path) {
        checkFileExists(path);
        return faabric::util::readFileToBytes(path);
    }
}
