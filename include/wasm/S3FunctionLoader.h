#pragma once

#include <aws/S3Wrapper.h>
#include <util/config.h>

#include "FunctionLoader.h"

namespace wasm {
    class S3FunctionLoader: public FunctionLoader {
    public:
        S3FunctionLoader();

        std::vector<uint8_t> loadFunctionBytes(const message::Message &msg);

        std::vector<uint8_t> loadFunctionObjectBytes(const message::Message &msg);

        void uploadFunction(message::Message &msg);

        void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes);
    private:
        util::SystemConfig &conf;
        awswrapper::S3Wrapper &s3;
    };
};