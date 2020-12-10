#include "FileserverFileLoader.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/http.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>
#include <pistache/http_header.h>

namespace storage {
std::string FileserverFileLoader::getFileserverUrl()
{
    return faabric::util::getSystemConfig().fileserverUrl;
}

std::vector<uint8_t> _doLoad(const std::string& url,
                             const std::string& path,
                             const std::string& storagePath)
{
    auto logger = faabric::util::getLogger();

    // Shortcut if already exists
    if (boost::filesystem::exists(storagePath)) {
        if (boost::filesystem::is_directory(storagePath)) {
            throw SharedFileIsDirectoryException(storagePath);
        } else {
            logger->debug("Loading from filesystem at {}", storagePath);
            return faabric::util::readFileToBytes(storagePath);
        }
    }

    logger->debug("Loading from fileserver: {} at {}", path, url);

    std::vector<uint8_t> fileBytes;
    if (path.empty()) {
        fileBytes = faabric::util::readFileFromUrl(url);
    } else {
        fileBytes = faabric::util::readFileFromUrlWithHeader(
          url, std::make_shared<FilePath>(path));
    }

    // Check response data
    if (fileBytes.empty()) {
        std::string errMsg = "Empty response for file " + path + " at " + url;
        logger->error(errMsg);
        throw faabric::util::InvalidFunctionException(errMsg);
    }

    // Check whether it's a directory
    // Note - we don't want to convert every file response to a string, so check
    // the length first
    std::string isDirResponse = IS_DIR_RESPONSE;
    if (fileBytes.size() == isDirResponse.size()) {
        std::string actualResp = faabric::util::bytesToString(fileBytes);
        if (actualResp == IS_DIR_RESPONSE) {
            throw SharedFileIsDirectoryException(path);
        }
    }

    // Write to file
    logger->debug("Writing file to filesystem at {}", storagePath);
    faabric::util::writeBytesToFile(storagePath, fileBytes);

    return fileBytes;
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWasm(
  const faabric::Message& msg)
{
    std::string url = faabric::util::getFunctionUrl(msg);
    std::string filePath = faabric::util::getFunctionFile(msg);
    return _doLoad(url, "", filePath);
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectFile(
  const faabric::Message& msg)
{
    std::string url = faabric::util::getFunctionObjectUrl(msg);
    std::string objectFilePath = faabric::util::getFunctionObjectFile(msg);
    return _doLoad(url, "", objectFilePath);
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWamrAotFile(
  const faabric::Message& msg)
{
    throw std::runtime_error(
      "Not yet implemented WAMR AOT file loading from fileserver");
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectFile(
  const std::string& path)
{
    std::string url = faabric::util::getSharedObjectObjectUrl();
    std::string objFilePath = faabric::util::getSharedObjectObjectFile(path);
    return _doLoad(url, path, objFilePath);
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectWasm(
  const std::string& path)
{
    std::string url = faabric::util::getSharedObjectUrl();
    return _doLoad(url, path, path);
}

std::vector<uint8_t> FileserverFileLoader::loadSharedFile(
  const std::string& path)
{
    std::string url = faabric::util::getSharedFileUrl();
    const std::string fullPath = faabric::util::getSharedFileFile(path);
    return _doLoad(url, path, fullPath);
}

// --------------------------------------------------------
// TODO: implement hashing and hash storage in fileserver mode
// --------------------------------------------------------
std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectHash(
  const faabric::Message& msg)
{
    std::vector<uint8_t> empty;
    return empty;
}

std::vector<uint8_t> FileserverFileLoader::loadFunctionWamrAotHash(
  const faabric::Message& msg)
{
    std::vector<uint8_t> empty;
    return empty;
}

std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectHash(
  const std::string& path)
{
    std::vector<uint8_t> empty;
    return empty;
}

void FileserverFileLoader::uploadFunctionObjectHash(
  const faabric::Message& msg,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadFunctionWamrAotHash(
  const faabric::Message& msg,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadSharedObjectObjectHash(
  const std::string& path,
  const std::vector<uint8_t>& hash)
{}

void FileserverFileLoader::uploadSharedObjectAotHash(
  const std::string& path,
  const std::vector<uint8_t>& hash)
{}

}
