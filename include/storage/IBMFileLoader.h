#pragma once

#include "FileLoader.h"
#include <util/config.h>
#include <ibm/IBMStorageWrapper.h>


namespace storage {
    class IBMFileLoader: public FileLoader {
    public:
        IBMFileLoader();

        std::vector<uint8_t> loadFunctionWasm(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectWasm(const std::string &path) override;

        std::vector<uint8_t> loadFunctionObjectFile(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectObjectFile(const std::string &path) override;

        std::vector<uint8_t> loadPythonFunctionFile(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedFile(const std::string &path) override {
            throw std::runtime_error("Not implemented for IBM function loader");
        }

        void uploadFunction(message::Message &msg) override;

        void uploadPythonFunction(message::Message &msg) override;

        void uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedFile(const std::string &path, const std::vector<uint8_t> &fileBytes) override {
            throw std::runtime_error("Not implemented for IBM function loader");
        }
    private:
        util::SystemConfig &conf;
        ibm::IBMStorageWrapper storageWrapper;

        std::vector<uint8_t> loadFileBytes(const std::string &path);
    };
};