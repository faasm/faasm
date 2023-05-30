#include <codegen/MachineCodeGenerator.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <openssl/evp.h>
#include <stdexcept>

using namespace faabric::util;

namespace codegen {

MachineCodeGenerator& getMachineCodeGenerator()
{
    static thread_local MachineCodeGenerator gen;
    return gen;
}

MachineCodeGenerator& getMachineCodeGenerator(storage::FileLoader& loaderIn)
{
    static thread_local MachineCodeGenerator gen(loaderIn);
    return gen;
}

MachineCodeGenerator::MachineCodeGenerator()
  : conf(conf::getFaasmConfig())
  , loader(storage::getFileLoader())
{}

MachineCodeGenerator::MachineCodeGenerator(storage::FileLoader& loaderIn)
  : conf(conf::getFaasmConfig())
  , loader(loaderIn)
{}

std::vector<uint8_t> MachineCodeGenerator::hashBytes(
  const std::vector<uint8_t>& bytes)
{
    // Use the new high-level hashing APIs as suggested for OpenSSL 3.0
    // https://github.com/openssl/openssl/issues/12260
    EVP_MD_CTX* mdctx;
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
    EVP_DigestUpdate(mdctx, bytes.data(), bytes.size());

    unsigned int md5DigestLen = EVP_MD_size(EVP_md5());
    std::vector<uint8_t> result(md5DigestLen);
    EVP_DigestFinal_ex(mdctx, result.data(), &md5DigestLen);
    EVP_MD_CTX_free(mdctx);

    return result;
}

std::vector<uint8_t> MachineCodeGenerator::doCodegen(
  std::vector<uint8_t>& bytes)
{
    if (conf.wasmVm == "wamr") {
        return wasm::wamrCodegen(bytes, false);
    }

    if (conf.wasmVm == "sgx") {
        return wasm::wamrCodegen(bytes, true);
    }

    return wasm::wavmCodegen(bytes);
}

void MachineCodeGenerator::codegenForFunction(faabric::Message& msg, bool clean)
{
    std::vector<uint8_t> bytes = loader.loadFunctionWasm(msg);

    const std::string funcStr = funcToString(msg, false);

    if (bytes.empty()) {
        throw std::runtime_error("Loaded empty bytes for " + funcStr);
    }

    // Compare hashes
    std::vector<uint8_t> newHash = hashBytes(bytes);
    std::vector<uint8_t> oldHash;
    if (conf.wasmVm == "wamr" || conf.wasmVm == "sgx") {
        oldHash = loader.loadFunctionWamrAotHash(msg);
    } else if (conf.wasmVm == "wavm") {
        oldHash = loader.loadFunctionObjectHash(msg);
    } else {
        SPDLOG_ERROR("Unrecognised WASM VM during codegen: {}", conf.wasmVm);
        throw std::runtime_error("Unrecognised WASM VM");
    }

    // If we run the machine code generator with the 'clean' flag, we ignore
    // previously recorded hashes
    if (!clean && (!oldHash.empty()) && newHash == oldHash) {
        // Even if we skip the code generation step, we want to sync the latest
        // object file
        if (conf.wasmVm == "wamr" || conf.wasmVm == "sgx") {
            UNUSED(loader.loadFunctionWamrAotHash(msg));
        } else {
            UNUSED(loader.loadFunctionObjectFile(msg));
        }
        SPDLOG_DEBUG(
          "Skipping codegen for {} (WASM VM: {})", funcStr, conf.wasmVm);
        return;
    }

    if (oldHash.empty()) {
        SPDLOG_DEBUG(
          "No old hash found for {} (WASM VM: {})", funcStr, conf.wasmVm);
    } else if (clean) {
        SPDLOG_DEBUG(
          "Generating machine code for {} (WASM VM: {})", funcStr, conf.wasmVm);
    } else {
        SPDLOG_DEBUG(
          "Hashes differ for {} (WASM VM: {})", funcStr, conf.wasmVm);
    }

    // Run the actual codegen
    std::vector<uint8_t> objBytes;
    try {
        objBytes = doCodegen(bytes);
    } catch (std::runtime_error& ex) {
        SPDLOG_ERROR(
          "Codegen failed for {} (WASM VM: {})", funcStr, conf.wasmVm);
        throw ex;
    }

    // Upload the file contents and the hash
    if (conf.wasmVm == "wamr" || conf.wasmVm == "sgx") {
        loader.uploadFunctionWamrAotFile(msg, objBytes);
        loader.uploadFunctionWamrAotHash(msg, newHash);
    } else {
        loader.uploadFunctionObjectFile(msg, objBytes);
        loader.uploadFunctionObjectHash(msg, newHash);
    }
}

void MachineCodeGenerator::codegenForSharedObject(const std::string& inputPath,
                                                  bool clean)
{
    // Load the wasm
    std::vector<uint8_t> bytes = loader.loadSharedObjectWasm(inputPath);

    // Check the hash
    std::vector<uint8_t> newHash = hashBytes(bytes);
    std::vector<uint8_t> oldHash = loader.loadSharedObjectObjectHash(inputPath);

    if ((!oldHash.empty()) && newHash == oldHash && !clean) {
        // Even if we skip the code generation step, we want to sync the latest
        // shared object object file
        UNUSED(loader.loadSharedObjectObjectFile(inputPath));
        SPDLOG_DEBUG("Skipping codegen for {}", inputPath);
        return;
    }

    // Run the actual codegen
    std::vector<uint8_t> objBytes = doCodegen(bytes);

    // Do the upload
    if (conf.wasmVm == "wamr" || conf.wasmVm == "sgx") {
        throw std::runtime_error(
          "Codegen for shared objects not supported with WAMR");
    }

    loader.uploadSharedObjectObjectFile(inputPath, objBytes);
    loader.uploadSharedObjectObjectHash(inputPath, newHash);
}
}
