#include "LocalFileLoader.h"

#include <iostream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/logging.h>
#include <util/files.h>

#include <boost/filesystem.hpp>

namespace storage {
    std::vector<uint8_t> LocalFileLoader::loadFunctionWasm(const message::Message &msg) {
        std::string filePath = util::getFunctionFile(msg);
        return loadFileBytes(filePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadFunctionObjectFile(const message::Message &msg) {
        std::string objectFilePath = util::getFunctionObjectFile(msg);
        return loadFileBytes(objectFilePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedObjectObjectFile(const std::string &path) {
        std::string objFilePath = util::getSharedObjectObjectFile(path);
        return loadFileBytes(objFilePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedObjectWasm(const std::string &path) {
        return loadFileBytes(path);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedFile(const std::string &path) {
        const std::string fullPath = util::getSharedFileFile(path);

        if (!boost::filesystem::exists(fullPath)) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->debug("Local file loader could not find file at {}", fullPath);
            std::vector<uint8_t> empty;
            return empty;
        }

        if(boost::filesystem::is_directory(fullPath)) {
            throw SharedFileIsDirectoryException(fullPath);
        }

        return loadFileBytes(fullPath);
    }

    void LocalFileLoader::uploadFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(msg, false);

        // Here the msg input data is actually the file
        const std::string &fileBody = msg.inputdata();
        if (fileBody.empty()) {
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
        codegenForFunction(msg);
    }

    void LocalFileLoader::uploadPythonFunction(message::Message &msg) {
        const std::string &fileBody = msg.inputdata();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string outputFile = util::getPythonFunctionFile(msg);
        logger->debug("Uploading python file {} to {}", util::funcToString(msg, false), outputFile);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();
    }

    void LocalFileLoader::uploadFunctionObjectFile(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        std::string objFilePath = util::getFunctionObjectFile(msg);
        util::writeBytesToFile(objFilePath, objBytes);
    }

    void LocalFileLoader::uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        std::string outputPath = util::getSharedObjectObjectFile(path);
        util::writeBytesToFile(outputPath, objBytes);
    }

    void LocalFileLoader::uploadSharedFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        const std::string fullPath = util::getSharedFileFile(path);
        util::writeBytesToFile(fullPath, objBytes);
    }
}
