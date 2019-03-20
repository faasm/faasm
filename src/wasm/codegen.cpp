#include <wasm/WasmModule.h>
#include <util/logging.h>
#include <util/files.h>
#include <WAVM/WASM/WASM.h>
#include <wasm/FunctionLoader.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc != 2) {
        logger->error("Must provide input file");
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = inputFile + "_wasm.so";

    wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
    functionLoader.compileToObjectFile(inputFile, outputFile);
}