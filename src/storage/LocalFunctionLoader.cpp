#include "LocalFunctionLoader.h"

#include <iostream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/logging.h>
#include <util/files.h>

#include <boost/filesystem.hpp>

namespace storage {
    void checkFileExists(const std::string &path) {
        if(!boost::filesystem::exists(path)) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("File {} does not exist", path);
            throw std::runtime_error("Expected file does not exist");
        }
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionBytes(const message::Message &msg) {
        std::string filePath = util::getFunctionFile(msg);
        return this->loadFileBytes(filePath);
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFileBytes(const std::string &path) {
        checkFileExists(path);
        return util::readFileToBytes(path);
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        std::string objectFilePath = util::getFunctionObjectFile(msg);
        return this->loadFileBytes(objectFilePath);
    }

    std::vector<uint8_t> LocalFunctionLoader::loadPythonFunction(const message::Message &msg) {
        std::string path = util::getPythonFunctionFile(msg);
        return this->loadFileBytes(path);
    }

    void LocalFunctionLoader::uploadFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(msg);

        // Here the msg input data is actually the file
        const std::string &fileBody = msg.inputdata();
        if(fileBody.empty()) {
            logger->error("Uploaded empty file to {}", funcStr);
            throw std::runtime_error("Uploaded empty file");
        }

        logger->debug("Uploading wasm file {}", funcStr);
        std::string outputFile = util::getFunctionFile(msg);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        logger->debug("Generating object file for {}", funcStr);
        this->compileToObjectFile(msg);
    }

    void LocalFunctionLoader::uploadPythonFunction(message::Message & msg) {
        const std::string &fileBody = msg.inputdata();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->debug("Uploading python file {}", util::funcToString(msg));
        std::string outputFile = util::getPythonFunctionFile(msg);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();
    }

    void LocalFunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        std::string objFilePath = util::getFunctionObjectFile(msg);
        this->uploadObjectBytes(objFilePath, objBytes);
    }

    void LocalFunctionLoader::uploadObjectBytes(const std::string &path, const std::vector<uint8_t> &objBytes) {
        util::writeBytesToFile(path, objBytes);
    }

}
