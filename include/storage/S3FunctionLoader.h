#pragma once

#include <aws/S3Wrapper.h>
#include <util/config.h>

#include "FunctionLoader.h"

namespace storage {
    class S3FunctionLoader: public FunctionLoader {
    public:
        S3FunctionLoader();

        std::vector<uint8_t> loadFunctionWasm(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectWasm(const std::string &path) override;

        std::vector<uint8_t> loadFunctionObjectFile(const message::Message &msg) override;

        std::vector<uint8_t> loadSharedObjectObjectFile(const std::string &path) override;

        std::vector<uint8_t> loadPythonFunctionFile(const message::Message &msg) override;

        void uploadFunction(message::Message &msg) override;

        void uploadPythonFunction(message::Message &msg) override;

        void uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) override;

        void uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) override;
    private:
        util::SystemConfig &conf;
        awswrapper::S3Wrapper &s3;

        std::vector<uint8_t> loadFileBytes(const std::string &path);
    };
};