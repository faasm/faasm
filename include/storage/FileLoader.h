#pragma once

#include <conf/FaasmConfig.h>
#include <storage/S3Wrapper.h>

#include <faabric/util/config.h>
#include <faabric/util/exception.h>
#include <faabric/util/func.h>

#define EMPTY_FILE_RESPONSE "Empty response"
#define IS_DIR_RESPONSE "IS_DIR"
#define FILE_PATH_HEADER "FilePath"

#define SHARED_FILE_PREFIX "faasm://"

#define SHARED_OBJ_EXT ".o"

#define HASH_EXT ".md5"

#define PYTHON_USER "python"
#define PYTHON_FUNC "py_func"
#define PYTHON_FUNC_DIR "pyfuncs"

namespace storage {

class FileLoader
{
  public:
    FileLoader();

    FileLoader(bool useLocalFsCacheIn);

    void clearLocalCache();

    std::string getHashFilePath(const std::string& path);

    // ----- Function wasm -----
    std::string getFunctionFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionWasm(const faabric::Message& msg);

    void uploadFunction(faabric::Message& msg);

    // ----- Function object files -----
    std::string getFunctionObjectFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionObjectFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionObjectHash(const faabric::Message& msg);

    void uploadFunctionObjectFile(const faabric::Message& msg,
                                  const std::vector<uint8_t>& objBytes);

    void uploadFunctionObjectHash(const faabric::Message& msg,
                                  const std::vector<uint8_t>& hash);

    // ----- Function WAMR AoT files -----
    std::string getFunctionAotFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionWamrAotFile(const faabric::Message& msg);

    std::vector<uint8_t> loadFunctionWamrAotHash(const faabric::Message& msg);

    void uploadFunctionWamrAotFile(const faabric::Message& msg,
                                   const std::vector<uint8_t>& objBytes);

    void uploadFunctionWamrAotHash(const faabric::Message& msg,
                                   const std::vector<uint8_t>& hash);

    // ----- Encrypted function wasm -----
    std::string getEncryptedFunctionFile(const faabric::Message& msg);

    // ----- Function symbols -----
    std::string getFunctionSymbolsFile(const faabric::Message& msg);

    // ----- Shared object wasm -----
    std::vector<uint8_t> loadSharedObjectWasm(const std::string& path);

    // ----- Shared object object files -----
    std::string getSharedObjectObjectFile(const std::string& realPath);

    std::vector<uint8_t> loadSharedObjectObjectFile(const std::string& path);

    std::vector<uint8_t> loadSharedObjectObjectHash(const std::string& path);

    void uploadSharedObjectObjectFile(const std::string& path,
                                      const std::vector<uint8_t>& objBytes);

    void uploadSharedObjectObjectHash(const std::string& path,
                                      const std::vector<uint8_t>& hash);

    // ----- Shared files -----
    std::string getSharedFileFile(const std::string& path);

    std::vector<uint8_t> loadSharedFile(const std::string& path);

    void uploadSharedFile(const std::string& path,
                          const std::vector<uint8_t>& fileBytes);

    // ----- Python files -----
    std::string getPythonFunctionFile(const faabric::Message& msg);

    std::string getPythonFunctionFileSharedPath(const faabric::Message& msg);

    std::string getPythonRuntimeFunctionFile(const faabric::Message& msg);

    void uploadPythonFunction(faabric::Message& msg);

    void convertMessageToPython(faabric::Message& msg);

  private:
    conf::FaasmConfig& conf;
    storage::S3Wrapper s3;

    bool useLocalFsCache = true;

    std::vector<uint8_t> doCodegen(std::vector<uint8_t>& bytes,
                                   const std::string& fileName,
                                   bool isSgx = false);

    std::vector<uint8_t> loadFileBytes(const std::string& path,
                                       const std::string& localCachePath,
                                       bool tolerateMissing = false);

    std::vector<uint8_t> loadHashFileBytes(const std::string& path,
                                       const std::string& localCachePath);

    void uploadFileBytes(const std::string& path,
                         const std::string& localCachePath,
                         const std::vector<uint8_t>& bytes);

    void uploadHashFileBytes(const std::string& path,
                         const std::string& localCachePath,
                         const std::vector<uint8_t>& bytes);

    void uploadFileString(const std::string& path,
                          const std::string& localCachePath,
                          const std::string& bytes);
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

