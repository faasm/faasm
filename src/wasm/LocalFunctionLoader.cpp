#include "LocalFunctionLoader.h"

#include <iostream>
#include <fstream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

namespace wasm {
    std::vector<uint8_t> LocalFunctionLoader::loadFunctionBytes(const message::Message &msg) {
        std::string filePath = util::getFunctionFile(msg);
        std::vector<uint8_t> fileBytes = util::readFileToBytes(filePath);

        return fileBytes;
    }

    std::vector<uint8_t> LocalFunctionLoader::loadFunctionObjectBytes(const message::Message &msg) {
        std::string objectFilePath = util::getFunctionObjectFile(msg);
        std::vector<uint8_t> bytes = util::readFileToBytes(objectFilePath);

        return bytes;
    }

    void LocalFunctionLoader::uploadFunction(message::Message &msg) {
        // Here the msg input data is actually the file
        const std::string &fileBody = msg.inputdata();

        std::string outputFile = util::getFunctionFile(msg);
        std::ofstream out(outputFile);
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        this->compileToObjectFile(msg);
    }

    void LocalFunctionLoader::uploadObjectBytes(const message::Message &msg, const std::vector<uint8_t> &objBytes) {
        std::string objFilePath = util::getFunctionObjectFile(msg);
        util::writeBytesToFile(objFilePath, objBytes);
    }

}
