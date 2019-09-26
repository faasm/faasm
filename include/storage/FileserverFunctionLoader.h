#pragma once

#include "FunctionLoader.h"

/**
 * This is a class that overrides *only* the loading of wasm and object files.
 * Any write operations are disabled because they're not valid.
 *
 * It's a stop-gap for environments that don't have decent object stores available.
 */
namespace storage {
    class FileserverFunctionLoader : public FunctionLoader {
    public:
        std::string getFileserverUrl();

        std::vector<uint8_t> loadFunctionWasm(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectWasm(const std::string &path) override;

        std::vector<uint8_t> loadFunctionObjectFile(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectObjectFile(const std::string &path) override;

        std::vector<uint8_t> loadPythonFunctionFile(const message::Message &msg) override;

        void uploadFunction(message::Message &msg) override {
            throw std::runtime_error("Not implemented for fileserver function loader");
        }

        void uploadPythonFunction(message::Message &msg) override {
            throw std::runtime_error("Not implemented for fileserver function loader");
        }

        void uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) override {
            throw std::runtime_error("Not implemented for fileserver function loader");
        }

        void uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) override {
            throw std::runtime_error("Not implemented for fileserver function loader");
        }
    };
};