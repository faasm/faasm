#pragma once

#include <aws/S3Wrapper.h>

#include "FunctionLoader.h"

namespace wasm {
    class S3FunctionLoader: public FunctionLoader {
    public:
        S3FunctionLoader();

        std::vector<uint8_t> loadFunctionBytes(const message::Message &msg);

        void uploadFunction(message::Message &msg);
    protected:
        void uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes);
    private:
        awswrapper::S3Wrapper &s3;
    };
};