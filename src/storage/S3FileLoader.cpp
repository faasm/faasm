#include <storage/S3FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

namespace storage {

const static std::string funcFile = "function.wasm";
const static std::string objFile = "function.wasm.o";

std::string getFunctionKey(const faabric::Message& msg)
{
    std::string key = "wasm";

    key += "/";
    key += msg.user();
    key += "/";
    key += msg.function();
    key += "/";
    key += funcFile;

    return key;
}

std::string getFunctionObjectKey(const faabric::Message& msg)
{
    std::string key = "wasm";

    key += "/";
    key += msg.user();
    key += "/";
    key += msg.function();
    key += "/";
    key += objFile;

    return key;
}

S3FileLoader::S3FileLoader()
  : conf(conf::getFaasmConfig())
{}

std::vector<uint8_t> S3FileLoader::loadFileBytes(const std::string& path)
{
    SPDLOG_DEBUG("Loading bytes from {}/{}", conf.s3Bucket, path);

    const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.s3Bucket, path);

    return bytes;
}

std::vector<uint8_t> S3FileLoader::loadFunctionWasm(const faabric::Message& msg)
{
    const std::string key = getFunctionKey(msg);
    return this->loadFileBytes(key);
}

std::vector<uint8_t> S3FileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    const std::string key = getFunctionObjectKey(msg);
    return this->loadFileBytes(key);
}

void S3FileLoader::uploadFunction(faabric::Message& msg)
{
    // Note, when uploading, the input data is the function body
    const std::string& inputBytes = msg.inputdata();

    const std::string key = getFunctionKey(msg);
    s3.addKeyStr(conf.s3Bucket, key, inputBytes);

    // Build the object file from the file we've just received
    this->codegenForFunction(msg);
}

void S3FileLoader::uploadFunctionObjectFile(
  const faabric::Message& msg,
  const std::vector<uint8_t>& objBytes)
{
    const std::string key = getFunctionObjectKey(msg);

    SPDLOG_DEBUG(
      "Uploading function object bytes to {}/{}", conf.s3Bucket, key);

    s3.addKeyBytes(conf.s3Bucket, key, objBytes);
}

void S3FileLoader::uploadSharedObjectObjectFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    SPDLOG_DEBUG(
      "Uploading shared object object bytes to {}/{}", conf.s3Bucket, path);

    s3.addKeyBytes(conf.s3Bucket, path, objBytes);
}

// ----------------------------------------
// Unimplemented
// ----------------------------------------

std::vector<uint8_t> S3FileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::vector<uint8_t> empty;
    return empty;
}

void S3FileLoader::uploadFunctionObjectHash(const faabric::Message& msg,
                                            const std::vector<uint8_t>& hash)
{}
}
