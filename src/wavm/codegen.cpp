#include "WAVMWasmModule.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <faabric/util/files.h>

using namespace WAVM;


namespace wasm {
    std::vector<uint8_t> wavmCodegen(std::vector<uint8_t> &bytes) {
        IR::Module moduleIR;

        // Feature flags
        moduleIR.featureSpec.simd = true;

        if (faabric::util::isWasm(bytes)) {
            // Handle WASM
            WASM::LoadError loadError;
            bool success = WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR, &loadError);
            if (!success) {
                throw std::runtime_error(
                        fmt::format("Failed to parse wasm binary: {}", loadError.message)
                );
            }
        } else {
            std::vector<WAST::Error> parseErrors;
            bool success = WAST::parseModule((const char *) bytes.data(), bytes.size(), moduleIR, parseErrors);
            WAST::reportParseErrors("wast_file", (const char *) bytes.data(), parseErrors);

            if (!success) {
                throw std::runtime_error("Failed to parse wast file");
            }
        }

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);
        return objBytes;
    }
}
