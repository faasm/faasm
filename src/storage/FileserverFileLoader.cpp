#include "FileserverFileLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>
#include <util/logging.h>
#include <util/config.h>

#include <boost/filesystem.hpp>


namespace storage {
    std::string FileserverFileLoader::getFileserverUrl() {
        return util::getSystemConfig().fileserverUrl;
    }

    std::vector<uint8_t> _doLoad(const std::string &url, const std::string &path, const std::string &storagePath) {
        std::string header;
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Shortcut if already exists
        if(boost::filesystem::exists(storagePath)) {
            logger->debug("Loading from filesystem at {}", storagePath);
            return util::readFileToBytes(storagePath);
        }

        logger->debug("Loading from fileserver at {}", url);

        if (!path.empty()) {
            header = std::string(FILE_PATH_HEADER) + ":" + path;
        }

        std::vector<uint8_t> fileBytes;

        try {
            fileBytes = util::readFileFromUrlWithHeader(url, header);
        } catch(util::FileNotFoundAtUrlException &e) {
            // Leave empty if file not found
        }

        if (fileBytes.empty()) {
            std::string errMsg ="Empty response for file at " + url;
            logger->error(errMsg);
            throw util::InvalidFunctionException(errMsg);
        }

        // Write to file
        logger->debug("Writing file to filesystem at {}", storagePath);
        util::writeBytesToFile(storagePath, fileBytes);

        return fileBytes;
    }

    std::vector<uint8_t> FileserverFileLoader::loadFunctionWasm(const message::Message &msg) {
        std::string url = util::getFunctionUrl(msg);
        std::string filePath = util::getFunctionFile(msg);
        return _doLoad(url, "", filePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadFunctionObjectFile(const message::Message &msg) {
        std::string url = util::getFunctionObjectUrl(msg);
        std::string objectFilePath = util::getFunctionObjectFile(msg);
        return _doLoad(url, "", objectFilePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedObjectObjectFile(const std::string &path) {
        std::string url = util::getSharedObjectObjectUrl();
        std::string objFilePath = util::getSharedObjectObjectFile(path);
        return _doLoad(url, path, objFilePath);
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedObjectWasm(const std::string &path) {
        std::string url = util::getSharedObjectUrl();
        return _doLoad(url, path, path);
    }

    std::vector<uint8_t> FileserverFileLoader::loadPythonFunctionFile(const message::Message &msg) {
        std::string path = util::getPythonFunctionFile(msg);
        std::string url = util::getPythonFunctionUrl(msg);
        return _doLoad(url, "", path);
    }

    std::vector<uint8_t> FileserverFileLoader::loadSharedFile(const std::string &path) {
        std::string url = util::getSharedFileUrl();
        const std::string fullPath = util::getSharedFileFile(path);
        return _doLoad(url, path, fullPath);
    }
}