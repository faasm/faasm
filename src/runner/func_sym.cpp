#include <wasm/WasmModule.h>

#include <fstream>

#include <faabric/util/logging.h>
#include <faabric/util/files.h>
#include <iomanip>
#include <faabric/util/func.h>
#include <wavm/WAVMWasmModule.h>

int main(int argc, char *argv[]) {
    faabric::util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = faabric::util::getLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];

    faabric::Message call;
    call.set_user(user);
    call.set_function(function);

    logger->info("Disassembling function {}/{}", user, function);

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    std::string outPath = faabric::util::getFunctionSymbolsFile(call);
    std::ofstream outfile;
    outfile.open(outPath, std::ios::out);

    outfile << std::setw(20) << std::left << "FUNC_NAME" << "DISAS_NAME" << std::endl;
    for(auto const &p : disasMap) {
        outfile << std::setw(20) << std::left << p.first << p.second << std::endl;
    }

    outfile.close();

    logger->info("Output at {}", outPath);

    return 0;
}