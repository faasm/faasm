#include "FileserverFunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>
#include <util/logging.h>

namespace wasm {
    std::vector<uint8_t> _loadBytes(const std::string &url) {
        const std::vector<uint8_t> fileBytes = util::readFileFromUrl(url);

        if (fileBytes.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Empty response for file at {}", url);
            throw std::runtime_error("Empty response from URL");
        }
        return fileBytes;
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionBytes(const message::Message &msg) {
        std::string url = util::getFunctionUrl(msg);
        return _loadBytes(url);
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        std::string url = util::getFunctionObjectUrl(msg);
        return _loadBytes(url);
    }

    std::string FileserverFunctionLoader::getFileserverUrl() {
        return util::getSystemConfig().fileserverUrl;
    }
}