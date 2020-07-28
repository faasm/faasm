#pragma once

#include <util/config.h>
#include "FileLoader.h"

namespace storage {
    class LocalFileLoader : public FileLoader {
    public:
        std::vector<uint8_t> loadFunctionWasm(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectWasm(const std::string &path) override;

        std::vector<uint8_t> loadFunctionObjectFile(const message::Message &msg) override;

        std::vector<uint8_t> loadFunctionWamrAotFile(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectObjectFile(const std::string &path) override;

        std::vector<uint8_t> loadSharedFile(const std::string &path) override;

        void uploadFunction(message::Message &msg) override;

        void uploadPythonFunction(message::Message &msg) override;

        void uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) override;

        void uploadFunctionAotFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedObjectAotFile(const std::string &path, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedFile(const std::string &path, const std::vector<uint8_t> &fileBytes) override;
    };
};