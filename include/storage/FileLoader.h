#pragma once

#include <conf/FaasmConfig.h>
#include <storage/S3Wrapper.h>

#include <faabric/util/config.h>
#include <faabric/util/exception.h>
#include <faabric/util/func.h>

#define EMPTY_FILE_RESPONSE "Empty response"
#define IS_DIR_RESPONSE "IS_DIR"
#define FILE_PATH_HEADER "FilePath"

namespace storage {

class FileLoader
{
  public:
    FileLoader();

    FileLoader(bool useLocalFsCacheIn);

    std::vector<uint8_t> loadFunctionWasm(const faabric::Message& msg);

    std::vector<uint8_t> loadSharedObjectWasm(const std::string& path);

    std::vector<uint8_t> loadFunctionObjectFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionWamrAotFile(const faabric::Message& msg);

    std::vector<uint8_t> loadSharedObjectObjectFile(const std::string& path);

    std::vector<uint8_t> loadSharedFile(const std::string& path);

    std::vector<uint8_t> loadFunctionObjectHash(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionWamrAotHash(const faabric::Message& msg);

    std::vector<uint8_t> loadSharedObjectObjectHash(const std::string& path);

    void uploadFunctionObjectHash(const faabric::Message& msg,
                                  const std::vector<uint8_t>& hash);

    void uploadFunctionWamrAotHash(const faabric::Message& msg,
                                   const std::vector<uint8_t>& hash);

    void uploadSharedObjectObjectHash(const std::string& path,
                                      const std::vector<uint8_t>& hash);

    void uploadSharedObjectAotHash(const std::string& path,
                                   const std::vector<uint8_t>& hash);

    void uploadFunction(faabric::Message& msg);

    void uploadPythonFunction(faabric::Message& msg);

    void uploadFunctionObjectFile(const faabric::Message& msg,
                                  const std::vector<uint8_t>& objBytes);

    void uploadFunctionAotFile(const faabric::Message& msg,
                               const std::vector<uint8_t>& objBytes);

    void uploadSharedObjectObjectFile(const std::string& path,
                                      const std::vector<uint8_t>& objBytes);

    void uploadSharedObjectAotFile(const std::string& path,
                                   const std::vector<uint8_t>& objBytes);

    void uploadSharedFile(const std::string& path,
                          const std::vector<uint8_t>& fileBytes);

    void codegenForFunction(faabric::Message& msg);

    void codegenForSharedObject(const std::string& inputPath);

    void flushFunctionFiles();
  private:
    conf::FaasmConfig& conf;
    storage::S3Wrapper s3;

    bool useLocalFsCache = true;


    std::vector<uint8_t> doCodegen(std::vector<uint8_t>& bytes,
                                   const std::string& fileName,
                                   bool isSgx = false);

    std::vector<uint8_t> hashBytes(const std::vector<uint8_t>& bytes);

    std::vector<uint8_t> loadFileBytes(const std::string& path,
                       const std::string& localCachePath,
                       bool tolerateMissing = false);

    void uploadFileBytes(const std::string& path,
                         const std::vector<uint8_t>& bytes);

    void uploadFileString(const std::string& path, const std::string& bytes);
};

FileLoader& getFileLoader();

class SharedFileIsDirectoryException : public faabric::util::FaabricException
{
  public:
    explicit SharedFileIsDirectoryException(const std::string& filePath)
      : faabric::util::FaabricException(filePath + " is a directory")
    {}
};
}

