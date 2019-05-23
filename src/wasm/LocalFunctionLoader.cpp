#include "LocalFunctionLoader.h"

#include <iostream>
#include <fstream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/logging.h>
#include <util/files.h>

#include <boost/filesystem.hpp>

namespace wasm {
    void checkFileExists(const std::string &path) {
        if(!boost::filesystem::exists(path)) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("File {} does not exist", path);
            throw std::runtime_error("Expected file does not exist");
        }
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionBytes(const message::Message &msg) {
        std::string filePath = util::getFunctionFile(msg);
        return this->loadFunctionObjectBytes(filePath);
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionBytes(const std::string &path) {
        checkFileExists(path);
        std::vector<uint8_t> fileBytes = util::readFileToBytes(path);
        return fileBytes;
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        std::string objectFilePath = util::getFunctionObjectFile(msg);
        return this->loadFunctionObjectBytes(objectFilePath);
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionObjectBytes(const std::string &path) {
        checkFileExists(path);
        std::vector<uint8_t> bytes = util::readFileToBytes(path);
        return bytes;
    }

    void LocalFunctionLoader::uploadFunction(message::Message &msg) {
        // Here the msg input data is actually the file
        const std::string &fileBody = msg.inputdata();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->debug("Uploading wasm file {}", util::funcToString(msg));
        std::string outputFile = util::getFunctionFile(msg);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        logger->debug("Generating object file for {}", util::funcToString(msg));
        this->compileToObjectFile(msg);
    }

    void LocalFunctionLoader::uploadPythonFunction(message::Message & msg) {
        // msg input data is actually the file
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
