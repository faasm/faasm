#include "S3FunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

namespace wasm {
    S3FunctionLoader::S3FunctionLoader() : s3(awswrapper::S3Wrapper::getThreadLocal()) {

    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionBytes(const message::Message &msg) {
        // TODO - load from S3
        // TODO - cache locally on filesystem?
        std::vector<uint8_t> bytes;

        return bytes;
    }

    void S3FunctionLoader::uploadFunction(message::Message &msg) {
        // TODO - upload to S3
    }

    void S3FunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        // TODO - upload to S3
    }

}
