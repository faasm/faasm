#include "S3FunctionLoader.h"

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

namespace wasm {
    S3FunctionLoader::S3FunctionLoader() : conf(util::getSystemConfig()), s3(awswrapper::S3Wrapper::getThreadLocal()) {

    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionBytes(const message::Message &msg) {
        const std::string key = util::getFunctionKey(msg);
        return this->loadFunctionBytes(key);
    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionBytes(const std::string &path) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Loading function bytes from {}/{}", conf.bucketName, path);

        const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.bucketName, path);

        return bytes;
    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        const std::string key = util::getFunctionObjectKey(msg);
        return this->loadFunctionObjectBytes(key);
    }

    std::vector<uint8_t> S3FunctionLoader::loadFunctionObjectBytes(const std::string &path) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Loading object bytes from {}/{}", conf.bucketName, path);

        const std::vector<uint8_t> bytes = s3.getKeyBytes(conf.bucketName, path);
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

    void S3FunctionLoader::uploadPythonFunction(message::Message &msg) {
        throw std::runtime_error("Not yet implemented Python upload on S3");
    }

    void S3FunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        const std::string key = util::getFunctionObjectKey(msg);

        this->uploadObjectBytes(key, objBytes);
    }

    void S3FunctionLoader::uploadObjectBytes(const std::string &path, const std::vector<uint8_t> &objBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Uploading object bytes to {}/{}", conf.bucketName, path);

        s3.addKeyBytes(conf.bucketName, path, objBytes);
    }

}
