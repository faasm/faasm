#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <storage/FileLoader.h>
#include <wavm/WAVMWasmModule.h>

#if (WAMR_EXECUTION_MODE_INTERP)
// Import for codegen not needed as it's not supported
#else
#include <wamr/WAMRWasmModule.h>
#endif

#include <stdexcept>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <openssl/md5.h>

namespace storage {

const static std::string funcFile = "function.wasm";
const static std::string objFile = "function.wasm.o";
const static std::string pyFile = "function.py";
const static std::string encryptedFuncFile = "function.wasm.enc";
const static std::string symFile = "function.symbols";
const static std::string wamrAotFile = "function.aot";
const static std::string sgxWamrAotFile = "function.aot.sgx";

void checkFileExists(const std::string& path)
{
    if (!boost::filesystem::exists(path)) {
        SPDLOG_ERROR("File {} does not exist", path);
        throw std::runtime_error("Expected file does not exist");
    }
}

std::string getKey(const faabric::Message& msg, const std::string& filename)
{
    std::string key = conf::getFaasmConfig().s3Bucket;

    key += "/";
    key += msg.user();
    key += "/";
    key += msg.function();
    key += "/";
    key += filename;

    return key;
}

FileLoader::FileLoader()
  : conf(conf::getFaasmConfig())
  , useLocalFsCache(true)
{}

FileLoader::FileLoader(bool useLocalFsCacheIn)
  : conf(conf::getFaasmConfig())
  , useLocalFsCache(useLocalFsCacheIn)
{}

std::vector<uint8_t> FileLoader::loadFileBytes(
  const std::string& path,
  const std::string& localCachePath,
  bool tolerateMissing)
{
    // Check locally first
    if (useLocalFsCache && boost::filesystem::exists(localCachePath)) {
        if (boost::filesystem::is_directory(localCachePath)) {
            throw SharedFileIsDirectoryException(localCachePath);
        } else {
            SPDLOG_DEBUG(
              "Loading {} from filesystem at {}", path, localCachePath);
            return faabric::util::readFileToBytes(localCachePath);
        }
    } else {
        // Load from S3
        SPDLOG_DEBUG("Loading bytes from S3 for {}/{}", conf.s3Bucket, path);
        std::vector<uint8_t> bytes =
          s3.getKeyBytes(conf.s3Bucket, path, tolerateMissing);
        faabric::util::writeBytesToFile(localCachePath, bytes);

        return bytes;
    }
}

void FileLoader::uploadFileBytes(const std::string& path,
                                 const std::vector<uint8_t>& bytes)
{
    SPDLOG_DEBUG(
      "Uploading {} bytes to {}/{}", bytes.size(), conf.s3Bucket, path);
    s3.addKeyBytes(conf.s3Bucket, path, bytes);
}

void FileLoader::uploadFileString(const std::string& path,
                                  const std::string& bytes)
{
    SPDLOG_DEBUG("Uploading {} bytes to {}/{} (string)",
                 bytes.size(),
                 conf.s3Bucket,
                 path);
    s3.addKeyStr(conf.s3Bucket, path, bytes);
}

std::vector<uint8_t> FileLoader::loadFunctionWasm(const faabric::Message& msg)
{
    const std::string key = getKey(msg, funcFile);
    return loadFileBytes(key, conf::getFunctionFile(msg));
}

std::vector<uint8_t> FileLoader::loadSharedObjectWasm(const std::string& path)
{
    return loadFileBytes(path, path);
}

std::vector<uint8_t> FileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, objFile);
    return loadFileBytes(key, conf::getFunctionObjectFile(msg));
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, wamrAotFile);
    return loadFileBytes(key, conf::getFunctionAotFile(msg));
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    return loadFileBytes(path, conf::getSharedObjectObjectFile(path));
}

std::vector<uint8_t> FileLoader::loadSharedFile(const std::string& path)
{
    return loadFileBytes(path, conf::getSharedFileFile(path));
}

std::vector<uint8_t> FileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::string cachePath = conf::getFunctionObjectFile(msg);
    std::string key = getKey(msg, objFile);
    return loadFileBytes(
      conf::getHashFilePath(key), conf::getHashFilePath(cachePath), true);
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    std::string cachePath = conf::getFunctionAotFile(msg);
    std::string key = getKey(msg, wamrAotFile);
    return loadFileBytes(
      conf::getHashFilePath(key), conf::getHashFilePath(cachePath), true);
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    const std::string hashPath = conf::getHashFilePath(path);
    return loadFileBytes(hashPath, hashPath);
}

void FileLoader::uploadFunctionObjectHash(const faabric::Message& msg,
                                          const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, objFile);
    key = conf::getHashFilePath(key);
    uploadFileBytes(key, hash);
}

void FileLoader::uploadFunctionWamrAotHash(const faabric::Message& msg,
                                           const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, wamrAotFile);
    key = conf::getHashFilePath(key);
    uploadFileBytes(key, hash);
}

void FileLoader::uploadSharedObjectObjectHash(const std::string& path,
                                              const std::vector<uint8_t>& hash)
{
    const std::string key = conf::getHashFilePath(path);
    uploadFileBytes(key, hash);
}

void FileLoader::uploadSharedObjectAotHash(const std::string& path,
                                           const std::vector<uint8_t>& hash)
{
    const std::string key = conf::getHashFilePath(path);
    uploadFileBytes(key, hash);
}

void FileLoader::uploadFunction(faabric::Message& msg)
{
    // Note, when uploading, the input data is the function body
    const std::string& inputBytes = msg.inputdata();

    const std::string key = getKey(msg, funcFile);
    uploadFileString(key, inputBytes);

    // Build the object file from the file we've just received
    codegenForFunction(msg);
}

void FileLoader::uploadPythonFunction(faabric::Message& msg)
{
    const std::string key = getKey(msg, pyFile);
    const std::string& inputBytes = msg.inputdata();
    uploadFileString(key, inputBytes);
}

void FileLoader::uploadFunctionObjectFile(const faabric::Message& msg,
                                          const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    uploadFileBytes(key, objBytes);
}

void FileLoader::uploadFunctionAotFile(const faabric::Message& msg,
                                       const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    uploadFileBytes(key, objBytes);
}

void FileLoader::uploadSharedObjectObjectFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    uploadFileBytes(path, objBytes);
}

void FileLoader::uploadSharedObjectAotFile(const std::string& path,
                                           const std::vector<uint8_t>& objBytes)
{
    uploadFileBytes(path, objBytes);
}

void FileLoader::uploadSharedFile(const std::string& path,
                                  const std::vector<uint8_t>& fileBytes)
{
    uploadFileBytes(path, fileBytes);
}

void FileLoader::flushFunctionFiles()
{
    // Note that here we are just flushing our local copies of the function
    // files, not those stored in S3.
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    // Nuke the function directory
    boost::filesystem::remove_all(conf.functionDir);

    // Nuke the machine code directory
    boost::filesystem::remove_all(conf.objectFileDir);
}

std::vector<uint8_t> FileLoader::hashBytes(const std::vector<uint8_t>& bytes)
{
    std::vector<uint8_t> result(MD5_DIGEST_LENGTH);
    MD5(reinterpret_cast<const unsigned char*>(bytes.data()),
        bytes.size(),
        result.data());

    return result;
}

std::vector<uint8_t> FileLoader::doCodegen(std::vector<uint8_t>& bytes,
                                           const std::string& fileName,
                                           bool isSgx)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.wasmVm == "wamr") {
#if (WAMR_EXECTION_MODE_INTERP)
        throw std::runtime_error(
          "WAMR codegen not supported with WAMR interp mode");
#else
        return wasm::wamrCodegen(bytes, isSgx);
#endif
    } else {
        assert(isSgx == false);
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
    } else if (conf.wasmVm == "wavm" && msg.issgx()) {
        SPDLOG_ERROR("Can't run SGX codegen for WAVM. Only WAMR is supported.");
        throw std::runtime_error("SGX codegen for WAVM");
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
        objBytes = doCodegen(bytes, funcStr, msg.issgx());
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

FileLoader& getFileLoader()
{
    static thread_local FileLoader fl;
    return fl;
}
}
