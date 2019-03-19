#include <wasm/WasmModule.h>
#include <util/logging.h>
#include <util/files.h>
#include <WAVM/WASM/WASM.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc != 2) {
        logger->error("Must provide input file");
        return 1;
    }

    std::string inputFile = argv[0];
    std::string outputFile = inputFile + "_wasm.so";

    logger->info("Writing machine code for {} to {}", inputFile, outputFile);

    IR::Module moduleIR;
    const std::vector<uint8_t> bytes = util::readFileToBytes(inputFile);

    bool success = WASM::loadBinaryModule(bytes.data(), bytes.size(), moduleIR);
    if (!success) {
        logger->error("Failed to parse wasm binary from {}", inputFile);
        return 1;
    }

    // Compile to object code
    Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
    std::vector<uint8_t> objBytes = Runtime::getObjectCode(module);

    util::writeBytesToFile(outputFile, objBytes);
}