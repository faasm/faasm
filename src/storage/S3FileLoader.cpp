#include "conf/function_utils.h"
#include <conf/FaasmConfig.h>
#include <storage/S3FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>

namespace storage {

const static std::string funcFile = "function.wasm";
const static std::string objFile = "function.wasm.o";
const static std::string pyFile = "function.py";
const static std::string encryptedFuncFile = "function.wasm.enc";
const static std::string symFile = "function.symbols";
const static std::string wamrAotFile = "function.aot";
const static std::string sgxWamrAotFile = "function.aot.sgx";

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

S3FileLoader::S3FileLoader()
  : conf(conf::getFaasmConfig())
{}

std::vector<uint8_t> S3FileLoader::loadFileBytes(
  const std::string& path,
  const std::string& localCachePath)
{
    // Check locally first
    if (boost::filesystem::exists(localCachePath)) {
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
        std::vector<uint8_t> bytes = s3.getKeyBytes(conf.s3Bucket, path);
        faabric::util::writeBytesToFile(localCachePath, bytes);
        return bytes;
    }
}

void S3FileLoader::uploadFileBytes(const std::string& path,
                                   const std::vector<uint8_t>& bytes)
{
    SPDLOG_DEBUG(
      "Uploading {} bytes to {}/{}", bytes.size(), conf.s3Bucket, path);
    s3.addKeyBytes(conf.s3Bucket, path, bytes);
}

void S3FileLoader::uploadFileString(const std::string& path,
                                    const std::string& bytes)
{
    SPDLOG_DEBUG("Uploading {} bytes to {}/{} (string)",
                 bytes.size(),
                 conf.s3Bucket,
                 path);
    s3.addKeyStr(conf.s3Bucket, path, bytes);
}

std::vector<uint8_t> S3FileLoader::loadFunctionWasm(const faabric::Message& msg)
{
    const std::string key = getKey(msg, funcFile);
    return loadFileBytes(key, conf::getFunctionFile(msg));
}

std::vector<uint8_t> S3FileLoader::loadSharedObjectWasm(const std::string& path)
{
    return loadFileBytes(path, path);
}

std::vector<uint8_t> S3FileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, objFile);
    return loadFileBytes(key, conf::getFunctionObjectFile(msg));
}

std::vector<uint8_t> S3FileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, wamrAotFile);
    return loadFileBytes(key, conf::getFunctionAotFile(msg));
}

std::vector<uint8_t> S3FileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    return loadFileBytes(path, conf::getSharedObjectObjectFile(path));
}

std::vector<uint8_t> S3FileLoader::loadSharedFile(const std::string& path)
{
    return loadFileBytes(path, conf::getSharedFileFile(path));
}

std::vector<uint8_t> S3FileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::string cachePath = conf::getFunctionObjectFile(msg);
    std::string key = getKey(msg, objFile);
    return loadFileBytes(getHashFilePath(key), getHashFilePath(cachePath));
}

std::vector<uint8_t> S3FileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    std::string cachePath = conf::getFunctionAotFile(msg);
    std::string key = getKey(msg, wamrAotFile);
    return loadFileBytes(getHashFilePath(key), getHashFilePath(cachePath));
}

std::vector<uint8_t> S3FileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    const std::string hashPath = getHashFilePath(path);
    return loadFileBytes(hashPath, hashPath);
}

void S3FileLoader::uploadFunctionObjectHash(const faabric::Message& msg,
                                            const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, objFile);
    key = getHashFilePath(key);
    uploadFileBytes(key, hash);
}

void S3FileLoader::uploadFunctionWamrAotHash(const faabric::Message& msg,
                                             const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, wamrAotFile);
    key = getHashFilePath(key);
    uploadFileBytes(key, hash);
}

void S3FileLoader::uploadSharedObjectObjectHash(
  const std::string& path,
  const std::vector<uint8_t>& hash)
{
    const std::string key = getHashFilePath(path);
    uploadFileBytes(key, hash);
}

void S3FileLoader::uploadSharedObjectAotHash(const std::string& path,
                                             const std::vector<uint8_t>& hash)
{
    const std::string key = getHashFilePath(path);
    uploadFileBytes(key, hash);
}

void S3FileLoader::uploadFunction(faabric::Message& msg)
{
    // Note, when uploading, the input data is the function body
    const std::string& inputBytes = msg.inputdata();

    const std::string key = getKey(msg, funcFile);
    uploadFileString(key, inputBytes);

    // Build the object file from the file we've just received
    codegenForFunction(msg);
}

void S3FileLoader::uploadPythonFunction(faabric::Message& msg)
{
    const std::string key = getKey(msg, pyFile);
    const std::string& inputBytes = msg.inputdata();
    uploadFileString(key, inputBytes);
}

void S3FileLoader::uploadFunctionObjectFile(
  const faabric::Message& msg,
  const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    uploadFileBytes(key, objBytes);
}

void S3FileLoader::uploadFunctionAotFile(const faabric::Message& msg,
                                         const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    uploadFileBytes(key, objBytes);
}

void S3FileLoader::uploadSharedObjectObjectFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    uploadFileBytes(path, objBytes);
}

void S3FileLoader::uploadSharedObjectAotFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    uploadFileBytes(path, objBytes);
}

void S3FileLoader::uploadSharedFile(const std::string& path,
                                    const std::vector<uint8_t>& fileBytes)
{
    uploadFileBytes(path, fileBytes);
}

void S3FileLoader::flushFunctionFiles()
{
    // Note that here we are just flushing our local copies of the function
    // files, not those stored in S3.
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    // Nuke the function directory
    boost::filesystem::remove_all(conf.functionDir);

    // Nuke the machine code directory
    boost::filesystem::remove_all(conf.objectFileDir);
}
}
