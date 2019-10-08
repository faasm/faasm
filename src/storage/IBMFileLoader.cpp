#include "IBMFileLoader.h"

#include <util/func.h>
#include <util/files.h>
#include <util/logging.h>



namespace storage {
    IBMFileLoader::IBMFileLoader() : conf(util::getSystemConfig()),
        storageWrapper(conf.ibmApiKey) {

    }

    std::vector<uint8_t> IBMFileLoader::loadFileBytes(const std::string &path) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Loading bytes from {}/{}", conf.bucketName, path);

        return storageWrapper.getKeyBytes(conf.bucketName, path);
    }

    std::vector<uint8_t> IBMFileLoader::loadFunctionWasm(const message::Message &msg) {
        const std::string key = util::getFunctionKey(msg);
        return this->loadFileBytes(key);
    }

    std::vector<uint8_t> IBMFileLoader::loadSharedObjectWasm(const std::string &path) {
        throw std::runtime_error("Not implemented for IBM function loader");
    }

    std::vector<uint8_t> IBMFileLoader::loadFunctionObjectFile(const message::Message &msg) {
        const std::string key = util::getFunctionObjectKey(msg);
        return this->loadFileBytes(key);
    }

    std::vector<uint8_t> IBMFileLoader::loadSharedObjectObjectFile(const std::string &path) {
        throw std::runtime_error("Not implemented for IBM function loader");
    }

    std::vector<uint8_t> IBMFileLoader::loadPythonFunctionFile(const message::Message &msg) {
        throw std::runtime_error("Loading python functions not supported with IBM");
    }

    void IBMFileLoader::uploadFunction(message::Message &msg) {
        // Note, when uploading, the input data is the function body
        const std::string &inputBytes = msg.inputdata();

        const std::string key = util::getFunctionKey(msg);
        storageWrapper.addKeyStr(conf.bucketName, key, inputBytes);

        // Build the object file from the file we've just received
        this->codegenForFunction(msg);
    }

    void IBMFileLoader::uploadPythonFunction(message::Message &msg) {
        throw std::runtime_error("Not yet implemented Python upload on IBM");
    }

    void IBMFileLoader::uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        const std::string key = util::getFunctionObjectKey(msg);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Uploading function object bytes to {}/{}", conf.bucketName, key);

        storageWrapper.addKeyBytes(conf.bucketName, key, objBytes);
    }

    void IBMFileLoader::uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Uploading shared object object bytes to {}/{}", conf.bucketName, path);

        storageWrapper.addKeyBytes(conf.bucketName, path, objBytes);
    }

}
