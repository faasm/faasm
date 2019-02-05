#include "FunctionLoader.h"

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>

#include <iostream>
#include <fstream>

#include <util/func.h>
#include <util/bytes.h>
#include <util/files.h>

using namespace WAVM;

namespace wasm {
    void FunctionLoader::loadFunctionBytes(const message::Message &msg, IR::Module &outModule) {
        std::vector<U8> fileBytes;
        std::string filePath = util::getFunctionFile(msg);

        if (!loadFile(filePath.c_str(), fileBytes)) {
            throw std::runtime_error("Could not read binary data from file");
        }

        WASM::loadBinaryModule(fileBytes.data(), fileBytes.size(), outModule);
    }

    void FunctionLoader::uploadFunction(message::Message &msg) {
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

    void FunctionLoader::compileToObjectFile(message::Message &msg) {
        // Parse the wasm file to work out imports, function signatures etc.
        IR::Module moduleIR;
        this->loadFunctionBytes(msg, moduleIR);

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);

        std::string objFilePath = util::getFunctionObjectFile(msg);
        util::writeBytesToFile(objFilePath, objBytes);
    }

}
