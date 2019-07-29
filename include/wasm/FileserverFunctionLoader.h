#pragma once

#include "LocalFunctionLoader.h"

/**
 * This is a class that overrides *only* the loading of function wasm and object files.
 * The rest relies on the local filesystem.
 *
 * It's a stop-gap for environments that don't have decent object stores available.
 */
namespace wasm {
    class FileserverFunctionLoader : public LocalFunctionLoader {
    public:
        FileserverFunctionLoader();

        std::vector<uint8_t> loadFunctionBytes(const message::Message &msg);

        std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg);
    private:
        const std::string fileserverUrl;
    };
};