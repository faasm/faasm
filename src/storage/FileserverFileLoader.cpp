#include "FileserverFileLoader.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <boost/filesystem.hpp>

namespace storage {
std::string FileserverFileLoader::getFileserverUrl()
{
    return faabric::util::getSystemConfig().fileserverUrl;
}

std::vector<uint8_t> _doLoad(const std::string& url,
                             const std::string& path,
                             const std::string& storagePath)
{
    std::string header;
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

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

    if (!path.empty()) {
        header = std::string(FILE_PATH_HEADER) + ":" + path;
    }

    std::vector<uint8_t> fileBytes;

    try {
        fileBytes = faabric::util::readFileFromUrlWithHeader(url, header);
    } catch (faabric::util::FileNotFoundAtUrlException& e) {
        // Leave empty if file not found
    } catch (faabric::util::FileAtUrlIsDirectoryException& e) {
        // Throw exception if we're dealing with a directory
        throw SharedFileIsDirectoryException(path);
    }

    if (fileBytes.empty()) {
        std::string errMsg = "Empty response for file " + path + " at " + url;
        logger->error(errMsg);
        throw faabric::util::InvalidFunctionException(errMsg);
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
}
