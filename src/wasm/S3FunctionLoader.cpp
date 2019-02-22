#include "S3FunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

namespace wasm {
    S3FunctionLoader::S3FunctionLoader() : conf(util::getSystemConfig()), s3(awswrapper::S3Wrapper::getThreadLocal()) {

    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionBytes(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string key = util::getFunctionKey(msg);
        logger->debug("Loading function bytes from {}/{}", conf.bucketName, key);

        const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.bucketName, key);
        
        return bytes;
    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        const std::string key = util::getFunctionObjectKey(msg);
        const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.bucketName, key);

        return bytes;
    }

    void S3FunctionLoader::uploadFunction(message::Message &msg) {
        // Note, when uploading, the input data is the function body
        const std::string &inputBytes = msg.inputdata();

        const std::string key = util::getFunctionKey(msg);
        s3.addKeyStr(conf.bucketName, key, inputBytes);

        // Build the object file from the file we've just received
        this->compileToObjectFile(msg);
    }

    void S3FunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string key = util::getFunctionObjectKey(msg);
        logger->debug("Uploading object bytes to {}/{}", conf.bucketName, key);
        s3.addKeyBytes(conf.bucketName, key, objBytes);
    }

}
