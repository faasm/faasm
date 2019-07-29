#include "FileserverFunctionLoader.h"

#include <iostream>
#include <fstream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/logging.h>
#include <util/files.h>

namespace wasm {
    FileserverFunctionLoader::FileserverFunctionLoader() : fileserverUrl(util::getSystemConfig().fileserverUrl) {
        if (fileserverUrl.empty()) {
            throw std::runtime_error("Must provide a fileserver URL when running in fileserver mode");
        }
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