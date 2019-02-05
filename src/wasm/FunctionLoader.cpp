#include "FunctionLoader.h"

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>

#include <util/config.h>

using namespace WAVM;

namespace wasm {
    void FunctionLoader::compileToObjectFile(message::Message &msg) {
        // Parse the wasm file to work out imports, function signatures etc.
        IR::Module moduleIR;
        const std::vector<uint8_t> &bytes = this->loadFunctionBytes(msg);

        WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR);
        
        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);

        this->uploadObjectBytes(msg, objBytes);
    }
}
