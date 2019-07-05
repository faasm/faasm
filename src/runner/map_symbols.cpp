#include <wasm/WasmModule.h>

#include <fstream>

#include <util/logging.h>
#include <util/files.h>
#include <iomanip>
#include <util/func.h>

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    std::string user = argv[1];
    std::string function = argv[2];

    message::Message call;
    call.set_user(user);
    call.set_function(function);

    logger->info("Disassembling function {}/{}", user, function);

    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    std::string outPath = util::getFunctionSymbolsFile(call);
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