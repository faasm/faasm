#include "FileLoader.h"

#include <stdexcept>

#if (WAMR_EXECUTION_MODE_INTERP)
// Import for codegen not needed as it's not supported
#else
#include <wamr/WAMRWasmModule.h>
#endif

#include <conf/FaasmConfig.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <openssl/md5.h>

namespace storage {

std::vector<uint8_t> FileLoader::hashBytes(const std::vector<uint8_t>& bytes)
{
    std::vector<uint8_t> result(MD5_DIGEST_LENGTH);
    MD5(reinterpret_cast<const unsigned char*>(bytes.data()),
        bytes.size(),
        result.data());

    return result;
}

std::string FileLoader::getHashFilePath(const std::string& path)
{
    return path + HASH_EXT;
}

std::vector<uint8_t> FileLoader::doCodegen(std::vector<uint8_t>& bytes,
                                           const std::string& fileName)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.wasmVm == "wamr") {
#if (WAMR_EXECTION_MODE_INTERP)
        throw std::runtime_error(
          "WAMR codegen not supported with WAMR interp mode");
#else
        return wasm::wamrCodegen(bytes);
#endif
    } else {
        return wasm::wavmCodegen(bytes, fileName);
    }
}

void FileLoader::codegenForFunction(faabric::Message& msg)
{
    std::vector<uint8_t> bytes = loadFunctionWasm(msg);

    const std::string funcStr = faabric::util::funcToString(msg, false);

    if (bytes.empty()) {
        throw std::runtime_error("Loaded empty bytes for " + funcStr);
    }

    // Compare hashes
    std::vector<uint8_t> newHash = hashBytes(bytes);
    std::vector<uint8_t> oldHash;
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.wasmVm == "wamr") {
        oldHash = loadFunctionWamrAotHash(msg);
    } else {
        oldHash = loadFunctionObjectHash(msg);
    }

    if ((!oldHash.empty()) && newHash == oldHash) {
        SPDLOG_DEBUG("Skipping codegen for {}", funcStr);
        return;
    } else if (oldHash.empty()) {
        SPDLOG_DEBUG("No old hash found for {}", funcStr);
    } else {
        SPDLOG_DEBUG("Hashes differ for {}", funcStr);
    }

    // Run the actual codegen
    std::vector<uint8_t> objBytes;
    try {
        objBytes = doCodegen(bytes, funcStr);
    } catch (std::runtime_error& ex) {
        SPDLOG_ERROR("Codegen failed for " + funcStr);
        throw ex;
    }

    // Upload the file contents and the hash
    if (conf.wasmVm == "wamr") {
        uploadFunctionAotFile(msg, objBytes);
        uploadFunctionWamrAotHash(msg, newHash);
    } else {
        uploadFunctionObjectFile(msg, objBytes);
        uploadFunctionObjectHash(msg, newHash);
    }
}

void FileLoader::codegenForSharedObject(const std::string& inputPath)
{

    // Load the wasm
    std::vector<uint8_t> bytes = loadSharedObjectWasm(inputPath);

    // Check the hash
    std::vector<uint8_t> newHash = hashBytes(bytes);
    std::vector<uint8_t> oldHash = loadSharedObjectObjectHash(inputPath);

    if ((!oldHash.empty()) && newHash == oldHash) {
        SPDLOG_DEBUG("Skipping codegen for {}", inputPath);
        return;
    }

    // Run the actual codegen
    std::vector<uint8_t> objBytes = doCodegen(bytes, inputPath);

    // Do the upload
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.wasmVm == "wamr") {
        uploadSharedObjectAotFile(inputPath, objBytes);
        uploadSharedObjectAotHash(inputPath, newHash);
    } else {
        uploadSharedObjectObjectFile(inputPath, objBytes);
        uploadSharedObjectObjectHash(inputPath, newHash);
    }
}

void checkFileExists(const std::string& path)
{
    if (!boost::filesystem::exists(path)) {
        const std::shared_ptr<spdlog::logger>& logger =

          SPDLOG_ERROR("File {} does not exist", path);
        throw std::runtime_error("Expected file does not exist");
    }
}

std::vector<uint8_t> loadFileBytes(const std::string& path)
{
    checkFileExists(path);
    return faabric::util::readFileToBytes(path);
}
}
