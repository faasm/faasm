#include <conf/FaasmConfig.h>
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

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <openssl/md5.h>

using namespace faabric::util;

namespace storage {

const static std::string funcFile = "function.wasm";
const static std::string objFile = "function.wasm.o";
const static std::string pyFile = "function.py";
const static std::string encryptedFuncFile = "function.wasm.enc";
const static std::string symFile = "function.symbols";
const static std::string wamrAotFile = "function.aot";
const static std::string sgxWamrAotFile = "function.aot.sgx";

std::string trimLeadingSlashes(const std::string& pathIn)
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
        }

        SPDLOG_DEBUG("Loading {} from filesystem at {}", path, localCachePath);
        return readFileToBytes(localCachePath);
    }

    // Load from S3 if not found
    std::string pathCopy = trimLeadingSlashes(path);
    std::vector<uint8_t> bytes =
      s3.getKeyBytes(conf.s3Bucket, pathCopy, tolerateMissing);

    if (!bytes.empty() && useLocalFsCache) {
        SPDLOG_DEBUG("Caching S3 key {}/{} at {}",
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
        SPDLOG_DEBUG("Caching S3 key {}/{} at {}",
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
        SPDLOG_DEBUG("Caching S3 key {}/{} at {}",
                     conf.s3Bucket,
                     pathCopy,
                     localCachePath);
        writeBytesToFile(localCachePath, stringToBytes(bytes));
    }
}

std::vector<uint8_t> FileLoader::loadFunctionWasm(const faabric::Message& msg)
{
    const std::string key = getKey(msg, funcFile);
    return loadFileBytes(key, getFunctionFile(msg));
}

std::vector<uint8_t> FileLoader::loadSharedObjectWasm(const std::string& path)
{
    return loadFileBytes(path, path);
}

std::vector<uint8_t> FileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, objFile);
    return loadFileBytes(key, getFunctionObjectFile(msg));
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    const std::string key = getKey(msg, wamrAotFile);
    return loadFileBytes(key, getFunctionAotFile(msg));
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    return loadFileBytes(path, getSharedObjectObjectFile(path));
}

std::vector<uint8_t> FileLoader::loadSharedFile(const std::string& path)
{
    return loadFileBytes(path, getSharedFileFile(path));
}

std::vector<uint8_t> FileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::string key = getKey(msg, objFile);
    std::string cachePath = getFunctionObjectFile(msg);

    return loadFileBytes(
      getHashFilePath(key), getHashFilePath(cachePath), true);
}

std::vector<uint8_t> FileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    std::string key = getKey(msg, wamrAotFile);
    std::string cachePath = getFunctionAotFile(msg);

    return loadFileBytes(
      getHashFilePath(key), getHashFilePath(cachePath), true);
}

std::vector<uint8_t> FileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    const std::string hashPath = getHashFilePath(path);

    const std::string localCachePath =
      getHashFilePath(getSharedObjectObjectFile(path));

    return loadFileBytes(hashPath, localCachePath, true);
}

void FileLoader::uploadFunctionObjectHash(const faabric::Message& msg,
                                          const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, objFile);
    key = getHashFilePath(key);
    std::string cachePath = getFunctionObjectFile(msg);
    uploadFileBytes(key, cachePath, hash);
}

void FileLoader::uploadFunctionWamrAotHash(const faabric::Message& msg,
                                           const std::vector<uint8_t>& hash)
{
    std::string key = getKey(msg, wamrAotFile);
    std::string cachePath = getFunctionAotFile(msg);
    uploadFileBytes(getHashFilePath(key), getHashFilePath(cachePath), hash);
}

void FileLoader::uploadSharedObjectObjectHash(const std::string& path,
                                              const std::vector<uint8_t>& hash)
{
    std::string key = getHashFilePath(path);
    std::string localCachePath =
      getHashFilePath(getSharedObjectObjectFile(path));
    uploadFileBytes(key, localCachePath, hash);
}

void FileLoader::uploadFunction(faabric::Message& msg)
{
    // Note, when uploading, the input data is the function body
    const std::string& inputBytes = msg.inputdata();
    const std::string key = getKey(msg, funcFile);
    const std::string localCachePath = getFunctionFile(msg);
    uploadFileString(key, localCachePath, inputBytes);

    // Build the object file from the file we've just received
    codegenForFunction(msg);
}

void FileLoader::uploadPythonFunction(faabric::Message& msg)
{
    const std::string key = getKey(msg, pyFile);
    const std::string& inputBytes = msg.inputdata();
    uploadFileString(key, "", inputBytes);
}

void FileLoader::uploadFunctionObjectFile(const faabric::Message& msg,
                                          const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    const std::string localCachePath = getFunctionObjectFile(msg);
    uploadFileBytes(key, localCachePath, objBytes);
}

void FileLoader::uploadFunctionWamrAotFile(const faabric::Message& msg,
                                           const std::vector<uint8_t>& objBytes)
{
    const std::string key = getKey(msg, objFile);
    const std::string localCachePath = getFunctionAotFile(msg);
    uploadFileBytes(key, localCachePath, objBytes);
}

void FileLoader::uploadSharedObjectObjectFile(
  const std::string& path,
  const std::vector<uint8_t>& objBytes)
{
    const std::string localCachePath = getSharedObjectObjectFile(path);
    uploadFileBytes(path, localCachePath, objBytes);
}

void FileLoader::uploadSharedFile(const std::string& path,
                                  const std::vector<uint8_t>& fileBytes)
{
    const std::string localCachePath = getSharedFileFile(path);
    uploadFileBytes(path, localCachePath, fileBytes);
}

void FileLoader::clearLocalCache()
{
    // Nuke the function directory
    SPDLOG_DEBUG("Clearing all files from {}", conf.functionDir);
    boost::filesystem::remove_all(conf.functionDir);

    // Nuke the machine code directory
    SPDLOG_DEBUG("Clearing all files from {}", conf.objectFileDir);
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

    const std::string funcStr = funcToString(msg, false);

    if (bytes.empty()) {
        throw std::runtime_error("Loaded empty bytes for " + funcStr);
    }

    // Compare hashes
    std::vector<uint8_t> newHash = hashBytes(bytes);
    std::vector<uint8_t> oldHash;
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
        uploadFunctionWamrAotFile(msg, objBytes);
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
    if (conf.wasmVm == "wamr") {
        throw std::runtime_error(
          "Codegen for shared objects not supported with WAMR");
    }

    uploadSharedObjectObjectFile(inputPath, objBytes);
    uploadSharedObjectObjectHash(inputPath, newHash);
}

std::string _doGetPythonFunctionFile(const faabric::Message& msg,
                                     const std::string& parentDir,
                                     bool createDirs)
{
    if (!msg.ispython()) {
        throw std::runtime_error(
          "Getting python function file for non-Python function " +
          funcToString(msg, false));
    }

    if (msg.pythonuser().empty() || msg.pythonfunction().empty()) {
        throw std::runtime_error(
          "Invalid Python call: user=" + msg.pythonuser() +
          " func=" + msg.pythonfunction());
    }
    boost::filesystem::path path(parentDir);

    path.append(PYTHON_FUNC_DIR);
    path.append(msg.pythonuser());
    path.append(msg.pythonfunction());

    if (createDirs) {
        boost::filesystem::create_directories(path);
    }

    path.append(pyFile);
    return path.string();
}

boost::filesystem::path _doGetDir(std::string baseDir,
                                  const faabric::Message& msg,
                                  bool create)
{
    boost::filesystem::path path(baseDir);
    path.append(msg.user());
    path.append(msg.function());

    // Create directory if doesn't exist
    if (create) {
        boost::filesystem::create_directories(path);
    }

    return path;
}

bool FileLoader::isValidFunction(const faabric::Message& msg)
{
    if (msg.user().empty() || msg.function().empty()) {
        return false;
    }

    auto path = _doGetDir(conf.functionDir, msg, false);
    path.append("function.wasm");

    bool isValid = boost::filesystem::exists(path);
    return isValid;
}

std::string FileLoader::getPythonFunctionFile(const faabric::Message& msg)
{
    // Python functions are stored as shared files to make it easier to
    // share them through the system
    return _doGetPythonFunctionFile(msg, conf.sharedFilesStorageDir, true);
}

std::string FileLoader::getPythonFunctionFileSharedPath(
  const faabric::Message& msg)
{
    // This is the shared path of the form faasm:// used to access the Python
    // file
    return _doGetPythonFunctionFile(msg, SHARED_FILE_PREFIX, false);
}

std::string FileLoader::getPythonRuntimeFunctionFile(
  const faabric::Message& msg)
{
    // This is the path where the file is placed at runtime to be
    // accessible to the function
    return _doGetPythonFunctionFile(msg, conf.runtimeFilesDir, true);
}

std::string FileLoader::getFunctionFile(const faabric::Message& msg)
{
    auto path = _doGetDir(conf.functionDir, msg, true);
    path.append(funcFile);

    return path.string();
}

std::string FileLoader::getEncryptedFunctionFile(const faabric::Message& msg)
{
    auto path = _doGetDir(conf.functionDir, msg, true);
    path.append(encryptedFuncFile);

    return path.string();
}

std::string FileLoader::getFunctionSymbolsFile(const faabric::Message& msg)
{
    auto path = _doGetDir(conf.functionDir, msg, true);
    path.append(symFile);

    return path.string();
}

std::string FileLoader::getFunctionObjectFile(const faabric::Message& msg)
{
    auto path = _doGetDir(conf.objectFileDir, msg, true);
    path.append(objFile);

    return path.string();
}

std::string FileLoader::getFunctionAotFile(const faabric::Message& msg)
{
    auto path = _doGetDir(conf.objectFileDir, msg, true);
    if (msg.issgx()) {
        path.append(sgxWamrAotFile);
    } else {
        path.append(wamrAotFile);
    }

    return path.string();
}

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

std::string FileLoader::getSharedFileFile(const std::string& path)
{
    boost::filesystem::path p(conf.sharedFilesStorageDir);
    p.append(path);

    if (!boost::filesystem::exists(p.parent_path())) {
        boost::filesystem::create_directories(p.parent_path());
    }

    return p.string();
}

void convertMessageToPython(faabric::Message& msg)
{
    msg.set_ispython(true);
    msg.set_pythonfunction(msg.function());
    msg.set_pythonuser(msg.user());

    msg.set_user(PYTHON_USER);
    msg.set_function(PYTHON_FUNC);
}

std::string getHashFilePath(const std::string& path)
{
    return path + HASH_EXT;
}

FileLoader& getFileLoader()
{
    static thread_local FileLoader fl;
    return fl;
}
}
