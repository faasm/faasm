#include "FileserverFileLoader.h"

#include <faabric/util/func.h>
#include <faabric/util/bytes.h>
#include <faabric/util/files.h>
#include <faabric/util/logging.h>
#include <faabric/util/config.h>

#include <boost/filesystem.hpp>


namespace storage {
    std::string FileserverFileLoader::getFileserverUrl() {
        return faabric::utilgetSystemConfig().fileserverUrl;
    }

    std::vector<uint8_t> _doLoad(const std::string &url, const std::string &path, const std::string &storagePath) {
        std::string header;
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();


        // Shortcut if already exists
        if(boost::filesystem::exists(storagePath)) {
            if(boost::filesystem::is_directory(storagePath)) {
                throw SharedFileIsDirectoryException(storagePath);
            } else {
                logger->debug("Loading from filesystem at {}", storagePath);
                return faabric::utilreadFileToBytes(storagePath);
            }
        }

        logger->debug("Loading from fileserver: {} at {}", path, url);

        if (!path.empty()) {
            header = std::string(FILE_PATH_HEADER) + ":" + path;
        }

        std::vector<uint8_t> fileBytes;

        try {
            fileBytes = faabric::utilreadFileFromUrlWithHeader(url, header);
        } catch(faabric::utilFileNotFoundAtUrlException &e) {
            // Leave empty if file not found
        } catch(faabric::utilFileAtUrlIsDirectoryException &e) {
            // Throw exception if we're dealing with a directory
            throw SharedFileIsDirectoryException(path);
        }

        if (fileBytes.empty()) {
            std::string errMsg ="Empty response for file " + path + " at " + url;
            logger->error(errMsg);
            throw faabric::utilInvalidFunctionException(errMsg);
        }

        // Write to file
        logger->debug("Writing file to filesystem at {}", storagePath);
        faabric::utilwriteBytesToFile(storagePath, fileBytes);

        return fileBytes;
    }

    std::vector<uint8_t> FileserverFileLoader::loadFunctionWasm(const faabric::Message &msg) {
        std::string url = faabric::utilgetFunctionUrl(msg);
        std::string filePath = faabric::utilgetFunctionFile(msg);
        return _doLoad(url, "", filePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectFile(const faabric::Message &msg) {
        std::string url = faabric::utilgetFunctionObjectUrl(msg);
        std::string objectFilePath = faabric::utilgetFunctionObjectFile(msg);
        return _doLoad(url, "", objectFilePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadFunctionWamrAotFile(const faabric::Message &msg) {
        throw std::runtime_error("Not yet implemented WAMR AOT file loading from fileserver");
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectFile(const std::string &path) {
        std::string url = faabric::utilgetSharedObjectObjectUrl();
        std::string objFilePath = faabric::utilgetSharedObjectObjectFile(path);
        return _doLoad(url, path, objFilePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedObjectWasm(const std::string &path) {
        std::string url = faabric::utilgetSharedObjectUrl();
        return _doLoad(url, path, path);
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedFile(const std::string &path) {
        std::string url = faabric::utilgetSharedFileUrl();
        const std::string fullPath = faabric::utilgetSharedFileFile(path);
        return _doLoad(url, path, fullPath);
    }
}