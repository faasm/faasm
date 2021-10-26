#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <storage/SharedFiles.h>

#include <stdexcept>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

using namespace faabric::util;

namespace storage {

// -------------------------------------
// UTILITIES
// -------------------------------------

#define FUNC_FILENAME "function.wasm"
#define FUNC_OBJECT_FILENAME "function.wasm.o"
#define PYTHON_FUNCTION_FILENAME "function.py"
#define FUNC_ENCRYPTED_FILENAME "function.wasm.enc"
#define FUNCTION_SYMBOLS_FILENAME "function.symbols"
#define WAMR_AOT_FILENAME "function.aot"
#define SGX_WAMR_AOT_FILENAME "function.aot.sgx"

static std::string trimLeadingSlashes(const std::string& pathIn)
{
    // Remove any leading slashes
    int startIdx = pathIn.find_first_not_of("/");

    // If there are no non-slashes return empty
    if (startIdx == std::string::npos) {
        return "";
    }

    if (startIdx == 0) {
        return pathIn;
    }

    std::string pathOut = pathIn.substr(startIdx);
    return pathOut;
}

static std::string getKey(const faabric::Message& msg,
                          const std::string& filename)
{
    std::string key =
      fmt::format("{}/{}/{}", msg.user(), msg.function(), filename);

    return key;
}

static boost::filesystem::path getDir(std::string baseDir,
                                      const faabric::Message& msg,
                                      bool create)
{
    boost::filesystem::path path(baseDir);
    path.append(msg.user());
    path.append(msg.function());

    // Create directory if required
    if (create) {
        boost::filesystem::create_directories(path);
    }

    return path;
}

// -------------------------------------
// MISC CLASS METHODS
// -------------------------------------

FileLoader::FileLoader()
  : conf(conf::getFaasmConfig())
  , useLocalFsCache(true)
{}

FileLoader::FileLoader(bool useLocalFsCacheIn)
  : conf(conf::getFaasmConfig())
  , useLocalFsCache(useLocalFsCacheIn)
{}

FileLoader& getFileLoader()
{
    static thread_local FileLoader fl(true);
    return fl;
}

FileLoader& getFileLoaderWithoutLocalCache()
{
    static thread_local FileLoader fl(false);
    return fl;
}

void FileLoader::clearLocalCache()
{
    if (faabric::util::isTestMode()) {
        SPDLOG_DEBUG("Not clearing local file loader cache in test mode");
        return;
    }

    SPDLOG_DEBUG("Clearing function wasm files from {}", conf.functionDir);
    boost::filesystem::remove_all(conf.functionDir);

    SPDLOG_DEBUG("Clearing function object files from {}", conf.objectFileDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    SPDLOG_DEBUG("Clearing shared files from {}", conf.sharedFilesDir);
    boost::filesystem::remove_all(conf.sharedFilesDir);

    SPDLOG_DEBUG("Clearing the local shared files cache");
    SharedFiles::clear();
}

// -------------------------------------
// SHARED LOAD/ UPLOAD
// -------------------------------------

std::vector<uint8_t> FileLoader::loadFileBytes(
  const std::string& path,
  const std::string& localCachePath,
  bool tolerateMissing)
{
    SPDLOG_TRACE("Loading file {} ({})", path, localCachePath);

    // Check locally first
    if (useLocalFsCache && boost::filesystem::exists(localCachePath)) {
        if (boost::filesystem::is_directory(localCachePath)) {
            throw SharedFileIsDirectoryException(localCachePath);
        }

        SPDLOG_TRACE("Loading {} from filesystem at {}", path, localCachePath);
        return readFileToBytes(localCachePath);
    }

    // Load from S3 if not found
    std::string pathCopy = trimLeadingSlashes(path);
    std::vector<uint8_t> bytes =
      s3.getKeyBytes(conf.s3Bucket, pathCopy, tolerateMissing);

    if (!bytes.empty() && useLocalFsCache) {
        SPDLOG_TRACE("Caching S3 key {}/{} at {}",
                     conf.s3Bucket,
                     pathCopy,
                     localCachePath);
        writeBytesToFile(localCachePath, bytes);
    }

    return bytes;
}

void FileLoader::uploadFileBytes(const std::string& path,
                                 const std::string& localCachePath,
                                 const std::vector<uint8_t>& bytes)
{
    std::string pathCopy = trimLeadingSlashes(path);
    s3.addKeyBytes(conf.s3Bucket, pathCopy, bytes);

    if (useLocalFsCache && !localCachePath.empty()) {
        SPDLOG_TRACE("Caching S3 key {}/{} at {}",
                     conf.s3Bucket,
                     pathCopy,
                     localCachePath);
        writeBytesToFile(localCachePath, bytes);
    }
}

void FileLoader::uploadFileString(const std::string& path,
                                  const std::string& localCachePath,
                                  const std::string& bytes)
{
    std::string pathCopy = trimLeadingSlashes(path);
    s3.addKeyStr(conf.s3Bucket, pathCopy, bytes);

    if (useLocalFsCache && !localCachePath.empty()) {
        SPDLOG_TRACE("Caching S3 key {}/{} at {}",
                     conf.s3Bucket,
                     pathCopy,
                     localCachePath);
        writeBytesToFile(localCachePath, stringToBytes(bytes));
    }
}

// -------------------------------------
// HASHING
// -------------------------------------

std::string FileLoader::getHashFilePath(const std::string& path)
{
    return path + HASH_EXT;
}

std::vector<uint8_t> FileLoader::loadHashFileBytes(
  const std::string& path,
  const std::string& localCachePath)
{
    return loadFileBytes(
      getHashFilePath(path), getHashFilePath(localCachePath), true);
}

void FileLoader::uploadHashFileBytes(const std::string& path,
                                     const std::string& localCachePath,
                                     const std::vector<uint8_t>& bytes)
{
    uploadFileBytes(
      getHashFilePath(path), getHashFilePath(localCachePath), bytes);
}

// -------------------------------------
// FUNCTION WASM
// -------------------------------------

std::string FileLoader::getFunctionFile(const faabric::Message& msg)
{
    auto path = getDir(conf.functionDir, msg, true);
    path.append(FUNC_FILENAME);
    return path.string();
}

std::vector<uint8_t> FileLoader::loadFunctionWasm(const faabric::Message& msg)
{
    const std::string key = getKey(msg, FUNC_FILENAME);
    const std::string localCachePath = getFunctionFile(msg);
    return loadFileBytes(key, localCachePath);
}

void FileLoader::uploadFunction(faabric::Message& msg)
{
    const std::string key = getKey(msg, FUNC_FILENAME);
    const std::string localCachePath = getFunctionFile(msg);

    // Note, when uploading, the input data is the function body
    const std::string& inputBytes = msg.inputdata();
    uploadFileString(key, localCachePath, inputBytes);
}

// -------------------------------------
// FUNCTION OBJECT FILES
// -------------------------------------

std::string FileLoader::getFunctionObjectFile(const faabric::Message& msg)
{
    auto path = getDir(conf.objectFileDir, msg, true);
    path.append(FUNC_OBJECT_FILENAME);
    return path.string();
}

std::vector<uint8_t> FileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, FUNC_OBJECT_FILENAME);
    const std::string localCachePath = getFunctionObjectFile(msg);
    return loadFileBytes(key, localCachePath);
}

std::vector<uint8_t> FileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, FUNC_OBJECT_FILENAME);
    const std::string localCachePath = getFunctionObjectFile(msg);
    return loadHashFileBytes(key, localCachePath);
}

void FileLoader::uploadFunctionObjectFile(const faabric::Message& msg,
                                          const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, FUNC_OBJECT_FILENAME);
    const std::string localCachePath = getFunctionObjectFile(msg);
    uploadFileBytes(key, localCachePath, objBytes);
}

void FileLoader::uploadFunctionObjectHash(const faabric::Message& msg,
                                          const std::vector<uint8_t>& hash)
{
    const std::string key = getKey(msg, FUNC_OBJECT_FILENAME);
    const std::string localCachePath = getFunctionObjectFile(msg);
    uploadHashFileBytes(key, localCachePath, hash);
}

// -------------------------------------
// FUNCTION WAMR AOT FILES
// -------------------------------------

static const std::string getWamrAotKey(const faabric::Message& msg)
{
    if (msg.issgx()) {
        return getKey(msg, SGX_WAMR_AOT_FILENAME);
    } else {
        return getKey(msg, WAMR_AOT_FILENAME);
    }
}

std::string FileLoader::getFunctionAotFile(const faabric::Message& msg)
{
    auto path = getDir(conf.objectFileDir, msg, true);
    if (msg.issgx()) {
        path.append(SGX_WAMR_AOT_FILENAME);
    } else {
        path.append(WAMR_AOT_FILENAME);
    }

    return path.string();
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    const std::string key = getWamrAotKey(msg);
    const std::string localCachePath = getFunctionAotFile(msg);
    return loadFileBytes(key, localCachePath);
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    const std::string key = getWamrAotKey(msg);
    const std::string localCachePath = getFunctionAotFile(msg);
    return loadHashFileBytes(key, localCachePath);
}

void FileLoader::uploadFunctionWamrAotFile(const faabric::Message& msg,
                                           const std::vector<uint8_t>& objBytes)
{
    const std::string key = getWamrAotKey(msg);
    const std::string localCachePath = getFunctionAotFile(msg);
    uploadFileBytes(key, localCachePath, objBytes);
}

void FileLoader::uploadFunctionWamrAotHash(const faabric::Message& msg,
                                           const std::vector<uint8_t>& hash)
{
    const std::string key = getWamrAotKey(msg);
    const std::string localCachePath = getFunctionAotFile(msg);
    uploadHashFileBytes(key, localCachePath, hash);
}

// -------------------------------------
// ENCRYPTED FUNCTION WASM
// -------------------------------------

std::string FileLoader::getEncryptedFunctionFile(const faabric::Message& msg)
{
    auto path = getDir(conf.functionDir, msg, true);
    path.append(FUNC_ENCRYPTED_FILENAME);
    return path.string();
}

// -------------------------------------
// FUNCTION SYMBOLS
// -------------------------------------

std::string FileLoader::getFunctionSymbolsFile(const faabric::Message& msg)
{
    auto path = getDir(conf.functionDir, msg, true);
    path.append(FUNCTION_SYMBOLS_FILENAME);
    return path.string();
}

// -------------------------------------
// SHARED OBJECT WASM
// -------------------------------------

std::vector<uint8_t> FileLoader::loadSharedObjectWasm(const std::string& path)
{
    return loadFileBytes(path, path);
}

// -------------------------------------
// SHARED OBJECT OBJECT FILES
// -------------------------------------

std::string FileLoader::getSharedObjectObjectFile(const std::string& realPath)
{
    boost::filesystem::directory_entry f(realPath);
    const std::string directory = f.path().parent_path().string();
    const std::string fileName = f.path().filename().string();

    // Work out the final destination for the object file. This will be the
    // object path with the directory of the original file appended
    boost::filesystem::path objPath(conf.objectFileDir);
    objPath.append(directory);

    // Create directory (if necessary)
    create_directories(objPath);

    // Add the filename
    std::string outputFile = objPath.append(fileName).string();
    outputFile += SHARED_OBJ_EXT;

    return outputFile;
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    const std::string localCachePath = getSharedObjectObjectFile(path);
    return loadFileBytes(path, localCachePath);
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    const std::string localCachePath = getSharedObjectObjectFile(path);
    return loadHashFileBytes(path, localCachePath);
}

void FileLoader::uploadSharedObjectObjectFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    const std::string localCachePath = getSharedObjectObjectFile(path);
    uploadFileBytes(path, localCachePath, objBytes);
}

void FileLoader::uploadSharedObjectObjectHash(const std::string& path,
                                              const std::vector<uint8_t>& hash)
{
    const std::string localCachePath = getSharedObjectObjectFile(path);
    uploadHashFileBytes(path, localCachePath, hash);
}

// -------------------------------------
// SHARED FILES
// -------------------------------------

std::string FileLoader::getSharedFileFile(const std::string& path)
{
    boost::filesystem::path p(conf.sharedFilesDir);
    p.append(path);

    // Create directories if necessary
    boost::filesystem::create_directories(p.parent_path());

    return p.string();
}

std::vector<uint8_t> FileLoader::loadSharedFile(const std::string& path)
{
    return loadFileBytes(path, getSharedFileFile(path));
}

void FileLoader::uploadSharedFile(const std::string& path,
                                  const std::vector<uint8_t>& fileBytes)
{
    const std::string localCachePath = getSharedFileFile(path);
    uploadFileBytes(path, localCachePath, fileBytes);
}

// -------------------------------------
// PYTHON FUNCTIONS
// -------------------------------------

std::string FileLoader::getPythonFunctionRelativePath(
  const faabric::Message& msg)
{
    std::string pythonUser = msg.pythonuser();
    std::string pythonFunction = msg.pythonfunction();

    if (pythonUser.empty() || pythonFunction.empty()) {
        std::string errorMsg =
          fmt::format("Accessing Python function without python user and "
                      "function set ({}/{})",
                      pythonUser,
                      pythonFunction);

        SPDLOG_ERROR(errorMsg);
        throw std::runtime_error(errorMsg);
    }

    boost::filesystem::path path(PYTHON_FUNC_DIR);
    path.append(pythonUser);
    path.append(pythonFunction);
    path.append(PYTHON_FUNCTION_FILENAME);
    return path.string();
}

std::string FileLoader::getPythonFunctionSharedFilePath(
  const faabric::Message& msg)
{
    // This is the shared path of the form faasm:// used to access the Python
    // file
    boost::filesystem::path path(SHARED_FILE_PREFIX);
    path.append(getPythonFunctionRelativePath(msg));
    return path.string();
}

std::string FileLoader::getPythonFunctionFile(const faabric::Message& msg)
{
    // This is the path where the file is actually stored to be available as a
    // shared file
    return getSharedFileFile(getPythonFunctionRelativePath(msg));
}

void FileLoader::uploadPythonFunction(faabric::Message& msg)
{
    // Note that Python functions are handled like shared files
    const std::string relativePath = getPythonFunctionRelativePath(msg);
    const std::string localCachePath = getSharedFileFile(relativePath);
    uploadFileString(relativePath, localCachePath, msg.inputdata());
}
}
