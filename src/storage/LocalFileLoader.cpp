#include "LocalFileLoader.h"

#include <iostream>

#include <faabric/util/func.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <faabric/util/files.h>

#include <boost/filesystem.hpp>

namespace storage {
    std::vector<uint8_t> LocalFileLoader::loadFunctionWasm(const faabric::Message &msg) {
        std::string filePath = faabric::util::getFunctionFile(msg);
        return loadFileBytes(filePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadFunctionObjectFile(const faabric::Message &msg) {
        std::string objectFilePath = faabric::util::getFunctionObjectFile(msg);
        return loadFileBytes(objectFilePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadFunctionWamrAotFile(const faabric::Message &msg) {
        std::string objectFilePath = faabric::util::getFunctionAotFile(msg);
        return loadFileBytes(objectFilePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedObjectObjectFile(const std::string &path) {
        std::string objFilePath = faabric::util::getSharedObjectObjectFile(path);
        return loadFileBytes(objFilePath);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedObjectWasm(const std::string &path) {
        return loadFileBytes(path);
    }

    std::vector<uint8_t> LocalFileLoader::loadSharedFile(const std::string &path) {
        const std::string fullPath = faabric::util::getSharedFileFile(path);

        if (!boost::filesystem::exists(fullPath)) {
            const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
            logger->debug("Local file loader could not find file at {}", fullPath);
            std::vector<uint8_t> empty;
            return empty;
        }

        if(boost::filesystem::is_directory(fullPath)) {
            throw SharedFileIsDirectoryException(fullPath);
        }

        return loadFileBytes(fullPath);
    }

    void LocalFileLoader::uploadFunction(faabric::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        const std::string funcStr = faabric::util::funcToString(msg, false);

        // Here the msg input data is actually the file
        const std::string &fileBody = msg.inputdata();
        if (fileBody.empty()) {
            logger->error("Uploaded empty file to {}", funcStr);
            throw std::runtime_error("Uploaded empty file");
        }

        logger->debug("Uploading wasm file {}", funcStr);
        std::string outputFile = faabric::util::getFunctionFile(msg);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        logger->debug("Generating object file for {}", funcStr);
        codegenForFunction(msg);
    }

    void LocalFileLoader::uploadPythonFunction(faabric::Message &msg) {
        const std::string &fileBody = msg.inputdata();
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

        // Message will have user/ function set as python user and python function
        faabric::util::convertMessageToPython(msg);

        std::string outputFile = faabric::util::getPythonFunctionFile(msg);
        logger->debug("Uploading python file {}/{} to {}", msg.pythonuser(), msg.pythonfunction(), outputFile);

        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();
    }

    void LocalFileLoader::uploadFunctionObjectFile(const faabric::Message &msg, const std::vector<uint8_t> &objBytes) {
        std::string objFilePath = faabric::util::getFunctionObjectFile(msg);
        faabric::util::writeBytesToFile(objFilePath, objBytes);
    }

    void LocalFileLoader::uploadFunctionAotFile(const faabric::Message &msg, const std::vector<uint8_t> &objBytes) {
        std::string objFilePath = faabric::util::getFunctionAotFile(msg);
        faabric::util::writeBytesToFile(objFilePath, objBytes);
    }

    void LocalFileLoader::uploadSharedObjectObjectFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        std::string outputPath = faabric::util::getSharedObjectObjectFile(path);
        faabric::util::writeBytesToFile(outputPath, objBytes);
    }

    void LocalFileLoader::uploadSharedObjectAotFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        throw std::runtime_error("Not yet implemented WAMR shared objects");
    }

    void LocalFileLoader::uploadSharedFile(const std::string &path, const std::vector<uint8_t> &objBytes) {
        const std::string fullPath = faabric::util::getSharedFileFile(path);
        faabric::util::writeBytesToFile(fullPath, objBytes);
    }
}
