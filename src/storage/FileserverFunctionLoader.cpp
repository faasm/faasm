#include "FileserverFunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>
#include <util/logging.h>
#include <util/config.h>


namespace storage {
    std::string FileserverFunctionLoader::getFileserverUrl() {
        return util::getSystemConfig().fileserverUrl;
    }

    std::vector<uint8_t> _loadBytesFromUrl(const std::string &url) {
        const std::vector<uint8_t> fileBytes = util::readFileFromUrl(url);

        if (fileBytes.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Empty response for file at {}", url);
            throw std::runtime_error("Empty response from URL");
        }
        return fileBytes;
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionWasm(const message::Message &msg) {
        std::string url = util::getFunctionUrl(msg);
        return _loadBytesFromUrl(url);
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionObjectFile(const message::Message &msg) {
        std::string url = util::getFunctionObjectUrl(msg);
        return _loadBytesFromUrl(url);
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadSharedObjectWasm(const std::string &path) {

    }

    std::vector<uint8_t> FileserverFunctionLoader::loadSharedObjectObjectFile(const std::string &path) {

    }

    std::vector<uint8_t> FileserverFunctionLoader::loadPythonFunctionFile(const message::Message &msg) {

    }
}