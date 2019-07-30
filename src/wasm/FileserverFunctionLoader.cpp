#include "FileserverFunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>
#include <util/logging.h>

namespace wasm {
    FileserverFunctionLoader::FileserverFunctionLoader() : fileserverUrl(util::getSystemConfig().fileserverUrl) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Fileserver loader running on {}", fileserverUrl);
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionBytes(const message::Message &msg) {
        std::string url = util::getFunctionUrl(msg);
        const std::vector<uint8_t> fileBytes = util::readFileFromUrl(url);
        return fileBytes;
    }

    std::vector<uint8_t> FileserverFunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        std::string url = util::getFunctionObjectUrl(msg);
        const std::vector<uint8_t> fileBytes = util::readFileFromUrl(url);
        return fileBytes;
    }
}