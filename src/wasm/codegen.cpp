#include <wasm/WasmModule.h>
#include <util/logging.h>
#include <util/files.h>
#include <WAVM/WASM/WASM.h>
#include <wasm/FunctionLoader.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->warn("No input provided to codegen");
        return 0;
    }

    std::string inputFile = argv[1];
    logger->info("Generating machine code for {}", inputFile);

    std::string outputFile = inputFile + SHARED_OBJ_EXT;

    wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
    functionLoader.compileToObjectFile(inputFile, outputFile);
}